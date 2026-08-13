"""Compile one demo_manager batch draft and diff every function it emits.

Deliberately extracts the TARGET side BY ADDRESS and asserts each body's
instruction count x 4 against the symbol map, because this project has had three
separate comparators return confident wrong answers. Also reports functions the
draft emits that the map does not name (invented names for file-statics), so
they can be mapped back to addresses at assembly instead of discarded as
"unmatched extras" -- discarding them desynchronised a positional comparison
once and produced dozens of spurious diffs.

    python wip/demo_manager/verify.py wip/demo_manager/dm-b3.cpp [--neg]

--neg runs a negative control: corrupts one instruction of the target side and
confirms the comparator actually reports a difference.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402

LO, HI = 0x8005B3A0, 0x8005D7E0
TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work',
                      'dol_bases_d_a_player_demo_manager', 'target.txt')
SYMS = os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')
SYM_RE = re.compile(
    r'(\S+)\s*=\s*\.text:0x([0-9A-Fa-f]{8}).*?type:function size:0x([0-9A-Fa-f]+)')


def map_functions():
    """-> [(addr, size, name)] for our range, in address order."""
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


def parse_by_addr(path):
    """-> {addr: [canonicalised instruction lines]} from a dtk disassembly."""
    fns, cur, addr, raw = {}, None, None, []
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = H.FN_START.match(s)
            if m:
                cur, addr, raw = m.group(1), None, []
                continue
            if H.FN_END.match(s):
                if cur and raw and addr is not None:
                    fns[addr] = H.canonicalise(raw)
                cur = None
                continue
            if cur is None:
                continue
            m = re.match(r'^/\*\s*([0-9A-F]{8})\s', s)
            if m and addr is None:
                addr = int(m.group(1), 16)
            m2 = H.INSN.match(s)
            if m2:
                raw.append(m2.group(1))
    return fns


def main():
    src = sys.argv[1]
    neg = '--neg' in sys.argv
    tag = os.path.splitext(os.path.basename(src))[0]
    scratch = os.path.join(os.path.dirname(os.path.abspath(src)), '_v_' + tag)
    obj, txt = scratch + '.o', scratch + '.txt'

    ok, log = H.compile_draft(src, obj)
    if not ok:
        print('COMPILE FAILED\n' + log[-3000:])
        return 1
    ok, log = H.disasm(obj, txt)
    if not ok:
        print('DISASM FAILED\n' + log[-1000:])
        return 1

    tgt = parse_by_addr(TARGET)
    drf_by_addr = parse_by_addr(txt)
    # the draft is a standalone object, so its addresses are file offsets, not
    # DOL addresses -- index it by NAME instead and pair via the symbol map.
    drf_by_name = {}
    with open(txt, encoding='utf-8', errors='replace') as fh:
        cur, raw = None, []
        for line in fh:
            s = line.strip()
            m = H.FN_START.match(s)
            if m:
                cur, raw = m.group(1), []
                continue
            if H.FN_END.match(s):
                if cur and raw:
                    drf_by_name[H.norm_name(cur)] = H.canonicalise(raw)
                cur = None
                continue
            if cur is None:
                continue
            m2 = H.INSN.match(s)
            if m2:
                raw.append(m2.group(1))

    exact = close = missing = 0
    print('%-52s %6s  %s' % ('function', 'bytes', 'result'))
    print('-' * 78)
    for a, size, name in map_functions():
        t = tgt.get(a)
        if t is None:
            continue
        if len(t) * 4 != size:
            print('%-52s %6d  !! TARGET count*4=%d != map size' %
                  (name[:52], size, len(t) * 4))
            continue
        d = drf_by_name.get(H.norm_name(name))
        if d is None:
            continue          # not this batch's function
        if t == d:
            print('%-52s %6d  MATCH' % (name[:52], size))
            exact += 1
        else:
            n = sum(1 for x, y in zip(t, d) if x != y) + abs(len(t) - len(d))
            print('%-52s %6d  DIFF  target=%d draft=%d  (~%d lines)' %
                  (name[:52], size, len(t), len(d), n))
            close += 1

    named = {H.norm_name(n) for _, _, n in map_functions()}
    extras = [n for n in drf_by_name if n not in named]
    if extras:
        print('\nEmitted but not named in the symbol map (invented names -- map these\n'
              'back to addresses at assembly, do NOT discard them):')
        for n in sorted(extras):
            print('   %s  (%d instructions)' % (n, len(drf_by_name[n])))

    print('\n%d exact, %d differing' % (exact, close))

    if neg:
        # negative control: the comparator must notice a corrupted target body
        for a, size, name in map_functions():
            key = H.norm_name(name)
            if key in drf_by_name and tgt.get(a) == drf_by_name[key]:
                bad = list(tgt[a])
                bad[0] = bad[0] + ' ;CORRUPTED'
                fired = (bad != drf_by_name[key])
                print('\nnegative control on %s: %s' %
                      (name, 'FIRED (comparator works)' if fired
                       else '!! DID NOT FIRE -- comparator is vacuous'))
                break
        else:
            print('\nnegative control: no matching function to corrupt')
    return 0


if __name__ == '__main__':
    sys.exit(main())
