"""Variant harness for one function of d_line_mng.cpp.

Reads the BASE draft, textually replaces the body of `line_cross_chk2` with a
candidate body read from a file, compiles, and reports the real (label-name and
pool-name insensitive) diff count against target, plus the side-by-side.

    python try.py <variant_body.txt> [--show]

The base file is never written.  Each variant compiles into its own _v/ dir.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SHARED = os.path.join(HERE, '..', 'line_mng_shared')
sys.path.insert(0, SHARED)
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools', 'auto_decomp'))
import harness
import tally

BASE = os.path.join(HERE, 'd_line_mng.cpp')
FN = 'line_cross_chk2'

# A label operand (.L_xxxx) and a pool symbol ("@nnnn"...) are named differently
# in a standalone object than in the target dump; neither is a real difference.
LBL = re.compile(r'\.L_[0-9A-Fa-f]+')
POOL = re.compile(r'"@[0-9]+(?:_[0-9A-Fa-f]+)?"')


def norm(t):
    return POOL.sub('"@P"', LBL.sub('.L', t))


def splice(base_text, new_body):
    """Replace the whole `bool dLineMng_c::line_cross_chk2(...) { ... }` block."""
    start = base_text.index('bool dLineMng_c::' + FN + '(')
    # find the opening brace of the definition, then match to its closer
    i = base_text.index('{', start)
    depth, j = 0, i
    while True:
        if base_text[j] == '{':
            depth += 1
        elif base_text[j] == '}':
            depth -= 1
            if depth == 0:
                break
        j += 1
    return base_text[:start] + new_body.rstrip() + '\n' + base_text[j + 1:]


def main():
    variant = os.path.abspath(sys.argv[1])
    show = '--show' in sys.argv
    tag = os.path.splitext(os.path.basename(variant))[0]

    base_text = open(BASE, encoding='utf-8').read()
    src_text = splice(base_text, open(variant, encoding='utf-8').read())

    work = os.path.join(HERE, '_v')
    os.makedirs(work, exist_ok=True)
    # The draft FILENAME is part of the object code (anonymous-namespace
    # mangling), so the variant must keep the real name.
    vdir = os.path.join(work, tag)
    os.makedirs(vdir, exist_ok=True)
    src = os.path.join(vdir, 'd_line_mng.cpp')
    open(src, 'w', encoding='utf-8', newline='\n').write(src_text)

    obj, txt = os.path.join(vdir, 'd.o'), os.path.join(vdir, 'd.txt')
    ok, err = harness.compile_draft(src, obj, extra_inc=[os.path.join(HERE, 'shadow_include')])
    if not ok:
        print('COMPILE FAILED\n' + err[-3000:])
        return 1
    harness.disasm(obj, txt)
    d, t = tally.parse(txt), tally.parse(tally.TARGET)
    dk = next(k for k in d if FN in k)
    tk = next(k for k in t if FN in k)
    dl, tl = d[dk], t[tk]

    n = 0
    lines = []
    for i in range(max(len(dl), len(tl))):
        dt = norm(dl[i][1]) if i < len(dl) else ''
        tt = norm(tl[i][1]) if i < len(tl) else ''
        raw_d = dl[i][1] if i < len(dl) else ''
        raw_t = tl[i][1] if i < len(tl) else ''
        m = ' ' if dt == tt else '*'
        if m == '*':
            n += 1
        lines.append(f'{i:4d} {m} {raw_d:<44} | {raw_t}')
    print(f'{tag}: {len(dl)}w vs {len(tl)}w   {n} REAL diffs')
    if show:
        print('\n'.join(lines))
    return 0


if __name__ == '__main__':
    sys.exit(main())
