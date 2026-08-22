"""Side-by-side of one function, draft against target, with a diff column.

`tally.py` gives a pass/fail per function; when a function is length-exact but
still unmatched you need to see WHICH words differ, not just that some do.
Reuses tally.py's parser so the two tools cannot disagree about what a function
is.

    python chk2_diff.py <symbol-substring> [draft.cpp] [shadow_include]
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SHARED = os.path.join(HERE, '..', 'line_mng_shared')
sys.path.insert(0, SHARED)
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools', 'auto_decomp'))
import harness
import tally


def main():
    want = sys.argv[1]
    src = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 else os.path.join(HERE, 'd_line_mng.cpp')
    inc = os.path.abspath(sys.argv[3]) if len(sys.argv) > 3 else os.path.join(HERE, 'shadow_include')

    work = os.path.join(HERE, '_tally')
    os.makedirs(work, exist_ok=True)
    obj, txt = os.path.join(work, 'd.o'), os.path.join(work, 'd.txt')
    ok, err = harness.compile_draft(src, obj, extra_inc=[inc])
    if not ok:
        print('COMPILE FAILED\n' + err)
        return 1
    harness.disasm(obj, txt)
    d, t = tally.parse(txt), tally.parse(tally.TARGET)

    dk = next((k for k in d if want in k), None)
    tk = next((k for k in t if want in k), None)
    if not dk or not tk:
        print(f'not found: draft={dk} target={tk}')
        return 1

    dl, tl = d[dk], t[tk]
    print(f'draft  {dk}  {len(dl)}w')
    print(f'target {tk}  {len(tl)}w\n')
    ndiff = 0
    for i in range(max(len(dl), len(tl))):
        db, dt = dl[i] if i < len(dl) else ('', '')
        tb, tt = tl[i] if i < len(tl) else ('', '')
        # Compare TEXT, not bytes: an address/pool-offset field is zeroed in
        # both disassemblies, so byte equality here would hide a real operand
        # difference rather than reveal it.
        mark = ' ' if dt == tt else '*'
        if mark == '*':
            ndiff += 1
        print(f'{i:4d} {mark} {dt:<44} | {tt}')
    print(f'\n{ndiff} differing of {len(tl)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
