"""Unit-level verifier for wip/player_manager/assembled.cpp, adapted from
tools/unit_verify.py (which targets the demo_manager unit's work/ dir).
Points TARGET straight at wip/player_manager/target_text.txt instead, since
this unit has no tools/auto_decomp/work/<unit>/ directory prepared.

    python verify_pm.py <assembled.cpp> [--neg] [--override DIR]
"""
import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402

LO = 0x8005E9A0
HI = 0x80061310
TARGET = os.path.join(ROOT, 'wip', 'player_manager', 'target_text.txt')
SYMS = os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')
SYM_RE = re.compile(
    r'(\S+)\s*=\s*\.text:0x([0-9A-Fa-f]{8}).*?type:function size:0x([0-9A-Fa-f]+)')


def map_functions():
    out = []
    with open(SYMS, encoding='utf-8') as fh:
        for line in fh:
            m = SYM_RE.search(line)
            if m:
                a = int(m.group(2), 16)
                if LO <= a < HI:
                    out.append((a, int(m.group(3), 16), m.group(1)))
    out.sort()
    return out


WORD_RE = re.compile(
    r'^/\*\s*([0-9A-F]{8})\s+[0-9A-F]{8}\s+((?:[0-9A-F]{2} ){3}[0-9A-F]{2})\s*\*/')


def parse_two_views(path):
    by_addr, by_name = {}, {}
    cur = addr = None
    lines, words = [], []
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = H.FN_START.match(s)
            if m:
                cur, addr, lines, words = m.group(1), None, [], []
                continue
            if H.FN_END.match(s):
                if cur and lines:
                    val = (H.canonicalise(lines), words)
                    if addr is not None:
                        by_addr[addr] = val
                    by_name[H.norm_name(cur)] = val
                cur = None
                continue
            if cur is None:
                continue
            mw = WORD_RE.match(s)
            if mw:
                if addr is None:
                    addr = int(mw.group(1), 16)
                words.append(mw.group(2).replace(' ', ''))
            m2 = H.INSN.match(s)
            if m2:
                lines.append(m2.group(1))
    return by_addr, by_name


def main():
    src = sys.argv[1]
    neg = '--neg' in sys.argv
    extra_inc = []
    if '--override' in sys.argv:
        extra_inc = [sys.argv[sys.argv.index('--override') + 1]]

    tag = os.path.splitext(os.path.basename(src))[0]
    scratch = os.path.join(os.path.dirname(os.path.abspath(src)), '_v_' + tag)
    obj, txt = scratch + '.o', scratch + '.txt'

    ok, log = H.compile_draft(src, obj, extra_inc=extra_inc)
    if not ok:
        print('COMPILE FAILED\n' + log[-6000:])
        return 1
    ok, log = H.disasm(obj, txt)
    if not ok:
        print('DISASM FAILED\n' + log[-1000:])
        return 1

    tgt, _ = parse_two_views(TARGET)
    _, drf = parse_two_views(txt)

    exact = close = branch_only = missing = 0
    print('%-52s %6s  %s' % ('function', 'bytes', 'result'))
    print('-' * 78)
    order_check = []
    for a, size, name in map_functions():
        t = tgt.get(a)
        if t is None:
            print('%-52s %6s  !! NOT IN target_text.txt AT THIS ADDRESS' % (name[:52], '?'))
            continue
        if len(t[0]) * 4 != size:
            print('%-52s %6d  !! TARGET count*4=%d != map size' %
                  (name[:52], size, len(t[0]) * 4))
        d = drf.get(H.norm_name(name))
        if d is None:
            print('%-52s %6d  !! MISSING FROM DRAFT' % (name[:52], size))
            missing += 1
            continue
        order_check.append((a, name))
        text_ok, word_ok = (t[0] == d[0]), (t[1] == d[1])
        if text_ok and word_ok:
            print('%-52s %6d  MATCH' % (name[:52], size))
            exact += 1
        elif text_ok and not word_ok:
            n = sum(1 for x, y in zip(t[1], d[1]) if x != y)
            print('%-52s %6d  !! BRANCH/WORD DIFF (%d words) -- canonical text '
                  'matches, raw words do NOT' % (name[:52], size, n))
            branch_only += 1
        else:
            n = sum(1 for x, y in zip(t[0], d[0]) if x != y) + abs(len(t[0]) - len(d[0]))
            print('%-52s %6d  DIFF  target=%d draft=%d  (~%d lines)' %
                  (name[:52], size, len(t[0]), len(d[0]), n))
            close += 1

    named = {H.norm_name(n) for _, _, n in map_functions()}
    extras = [n for n in drf if n not in named]
    if extras:
        print('\nEmitted but not named in the symbol map (invented names -- map these\n'
              'back to addresses at assembly, do NOT discard them):')
        for n in sorted(extras):
            print('   %s  (%d instructions)' % (n, len(drf[n][0])))

    # emission order check: walk the disasm txt in file order and compare to
    # address order.
    emitted_order = []
    with open(txt, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = H.FN_START.match(line.strip())
            if m and not m.group(1).startswith('gap_'):
                emitted_order.append(H.norm_name(m.group(1)))
    expect_order = [H.norm_name(n) for _, _, n in map_functions()]
    # filter both to only names present in both, preserving each side's order
    filt_emitted = [n for n in emitted_order if n in named]
    filt_expect = [n for n in expect_order if n in set(emitted_order)]
    if filt_emitted == filt_expect:
        print('\nEMISSION ORDER: matches target address order (%d functions)' % len(filt_emitted))
    else:
        print('\nEMISSION ORDER: !! MISMATCH vs target address order')
        for i, (a, b) in enumerate(zip(filt_emitted, filt_expect)):
            if a != b:
                print('   at position %d: draft has %s, expected %s' % (i, a, b))

    print('\n%d exact, %d differing, %d matching-text-but-wrong-branches, %d missing'
          % (exact, close, branch_only, missing))
    print('\n!! THIS TOOL CANNOT SEE A WRONG CONSTANT.')

    if neg:
        for a, size, name in map_functions():
            key = H.norm_name(name)
            if key in drf and tgt.get(a) and tgt[a] == drf[key]:
                bad_words = list(tgt[a][1])
                if bad_words:
                    bad_words[0] = '00000000'
                fired = (bad_words != drf[key][1])
                print('\nnegative control (raw words) on %s: %s' %
                      (name, 'FIRED' if fired else '!! DID NOT FIRE -- vacuous'))
                break
        else:
            print('\nnegative control: no matching function to corrupt')
    return 0


if __name__ == '__main__':
    sys.exit(main())
