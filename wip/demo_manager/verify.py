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


# dtk instruction line: /* ADDR OFFSET  AA BB CC DD */\tmnemonic ...
WORD_RE = re.compile(
    r'^/\*\s*([0-9A-F]{8})\s+[0-9A-F]{8}\s+((?:[0-9A-F]{2} ){3}[0-9A-F]{2})\s*\*/')


def parse_two_views(path):
    """-> {addr_or_name: (canonical_lines, raw_words)}.

    TWO views, because each is blind to what the other catches:

    * canonical text -- immune to the draft sitting at different addresses, but
      it rewrites branch TARGETS to a bare marker, so a `return` written where
      the target had `break` compares EQUAL. That happened on
      executeGoalCastle and the table reported a match.
    * raw instruction words -- position-independent for relative branches (the
      displacement is encoded in the word), and dtk zeroes relocated fields on
      both sides, so callee addresses do not leak in. This view sees branch
      targets; the canonical one does not.

    Neither view sees a wrong CONSTANT in a data table. Nothing here does.
    """
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

    tgt, _ = parse_two_views(TARGET)
    _, drf = parse_two_views(txt)

    exact = close = branch_only = 0
    print('%-52s %6s  %s' % ('function', 'bytes', 'result'))
    print('-' * 78)
    for a, size, name in map_functions():
        t = tgt.get(a)
        if t is None:
            continue
        if len(t[0]) * 4 != size:
            print('%-52s %6d  !! TARGET count*4=%d != map size' %
                  (name[:52], size, len(t[0]) * 4))
            continue
        d = drf.get(H.norm_name(name))
        if d is None:
            continue          # not this batch's function
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

    print('\n%d exact, %d differing, %d matching-text-but-wrong-branches'
          % (exact, close, branch_only))
    print('\n!! THIS TOOL CANNOT SEE A WRONG CONSTANT. It compares canonicalised\n'
          '   instruction text, so pool references are compared by PATTERN, not by\n'
          '   value -- a lone 0.0f and a lone 8.0f are equal here, and a corrupted\n'
          '   float inside a .rodata table changes NOTHING in this output. That was\n'
          '   demonstrated live on setHanabiEffect: 64.0f -> 65.0f in a table left\n'
          '   this report byte-for-byte identical. If your function owns data,\n'
          '   read the bytes out of the compiled object and compare them against\n'
          '   original/wiimj2d.dol separately. MATCH here is necessary, not\n'
          '   sufficient.')

    if neg:
        # negative control: the comparator must notice a corrupted target body
        for a, size, name in map_functions():
            key = H.norm_name(name)
            if key in drf and tgt.get(a) and tgt[a] == drf[key]:
                bad_words = list(tgt[a][1])
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
