"""A4 corpus verification: which source shape gives retail's MEMBER-in-slot-A fmuls?

Compiles wip/gapA/gapA_all.cpp with the `mSpeed.x = mBaseSpeed * 0.8910065f`
statement rewritten, and diffs executeState_Left30Left against retail.

Usage: python try_mul.py <variant> | --list
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'wip', 'gapA', 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
FN = 'executeState_Left30Left__10dLineMng_cFv'

# Only rewrite the two occurrences inside Left30Left (lines 1659 / 1665 in the
# merge draft). Anchor on the surrounding statements so the sibling states are
# left untouched -- this isolates the phenomenon.
ANCHOR_A = '''    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 16.0f) + 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = 0.5f * mSpeed.x;
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        // COPY-THEN-ADJUST'''


def make(stmt):
    return ANCHOR_A.replace(
        '    mSpeed.x = mBaseSpeed * 0.8910065f;\n',
        stmt.replace('\n', '\n') + '\n', 1
    ).replace(
        '        mSpeed.x = mBaseSpeed * 0.8910065f;\n',
        '\n'.join('    ' + l for l in stmt.split('\n')) + '\n', 1
    )


VARIANTS = {
    # control: the merge draft as-is (plain multiply, direct literal).
    'control': '    mSpeed.x = mBaseSpeed * 0.8910065f;',
    # R1: compound assignment (corpus route 1) -- the fix_bigtwo shape.
    'compound': '    mSpeed.x = mBaseSpeed;\n    mSpeed.x *= 0.8910065f;',
    # R3a: factor via a local variable (corpus route 3, cheapest form).
    'localvar': '    float rate = 0.8910065f;\n    mSpeed.x = mBaseSpeed * rate;',
    # R3b: factor via a const local variable.
    'constlocal': '    const float rate = 0.8910065f;\n    mSpeed.x = mBaseSpeed * rate;',
    # sanity: swapped operand order, must be identical to control.
    'swapped': '    mSpeed.x = 0.8910065f * mBaseSpeed;',
}


def parse(path):
    fns, cur = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"'); fns[cur] = []; continue
        if re.match(r'\s*\.endfn', line):
            cur = None; continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


def main():
    name = sys.argv[1]
    if name == '--list':
        print('\n'.join(VARIANTS)); return

    src = open(BASE, encoding='utf-8').read()
    if src.count(ANCHOR_A) != 1:
        sys.exit('ANCHOR count = %d' % src.count(ANCHOR_A))
    out_src = os.path.join(HERE, 'v_%s.cpp' % name)
    open(out_src, 'w', encoding='utf-8').write(src.replace(ANCHOR_A, make(VARIANTS[name])))

    obj = os.path.join(HERE, 'v_%s.o' % name)
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[INC])
    if not ok:
        sys.exit('COMPILE FAILED\n' + log[-3000:])
    txt = os.path.join(HERE, 'v_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        sys.exit('DISASM FAILED\n' + log[-2000:])

    draft = parse(txt).get(FN)
    target = parse(TARGET).get(FN)
    if draft is None:
        sys.exit('draft fn missing')
    same = sum(1 for a, b in zip(target, draft) if a[0] == b[0])
    print('variant %-12s target %d  draft %d  (%+d)  bytes-equal %d/%d'
          % (name, len(target), len(draft), len(draft) - len(target), same, len(target)))
    n = 0
    for i, (t, d) in enumerate(zip(target, draft)):
        if t[0] != d[0]:
            print('  %3d  T %-40s  D %s' % (i, t[1], d[1]))
            n += 1
            if n >= 12:
                print('  ...'); break
    if n == 0 and len(target) == len(draft):
        print('  *** EXACT MATCH ***')


main()
