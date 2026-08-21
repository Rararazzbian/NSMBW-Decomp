"""Gap A experiment driver.

Compiles a VARIANT of d_line_mng.cpp in which one function body has been
replaced, then diffs that single function against retail, instruction by
instruction. Prints target/draft lengths and the first divergence.

The point is to find the SOURCE SHAPE that makes MWCC reload a struct field
instead of reusing a value it still holds in a register -- without `volatile`.

Usage:  python try.py <variant_name>
        python try.py --list
"""
import os, re, sys, shutil

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'd_line_mng.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
FN = 'executeState_Left30Left__10dLineMng_cFv'

# `mVec2_c newBase(...)` alone appears twice -- Left30Left and Left60Down share
# it. Anchor on the mov_frm_leftlower call above it, which does not.
PREFIX = '''        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
'''
SUFFIX = '''        if (lineUnitNo == 8) {'''

# The body currently in the draft, verbatim -- the anchor we substitute for.
BODY = '''        mVec2_c newBase(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
'''
ORIG = PREFIX + BODY + SUFFIX

VARIANTS = {
    # 0: control -- unchanged source. Must reproduce the known -1 word gap.
    'control': BODY,

    # 1: copy-then-adjust. Initialising the local from the whole member forces
    #    a fresh load of BOTH fields; the += then re-adds. Predicts the target's
    #    reload + recompute + y-before-x store order.
    'copy_adjust': '''        mVec2_c newBase = mUnitBasePos;
        newBase.x += 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
''',

    # 2: same idea, field-by-field rather than whole-struct copy.
    'field_copy': '''        mVec2_c newBase;
        newBase.x = mUnitBasePos.x + 16.0f;
        newBase.y = mUnitBasePos.y;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
''',

    # 3: y first, then x -- tests whether the store order is a source-order
    #    effect independent of the reload.
    'y_first': '''        mVec2_c newBase;
        newBase.y = mUnitBasePos.y;
        newBase.x = mUnitBasePos.x + 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
''',

    # 4: no local at all -- pass the expressions straight through.
    'no_local': '''        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
        mVec2_c newBase(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
''',
}


def parse(path):
    fns, cur = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            fns[cur] = []
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


def main():
    name = sys.argv[1]
    if name == '--list':
        print('\n'.join(VARIANTS))
        return
    body = VARIANTS[name]

    src = open(BASE, encoding='utf-8').read()
    if src.count(ORIG) != 1:
        sys.exit('ANCHOR NOT UNIQUE: found %d occurrences' % src.count(ORIG))
    out_src = os.path.join(HERE, 'v_%s.cpp' % name)
    open(out_src, 'w', encoding='utf-8').write(src.replace(ORIG, PREFIX + body + SUFFIX))

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
    if draft is None or target is None:
        sys.exit('function missing: draft=%s target=%s' % (draft is None, target is None))

    print('variant  : %s' % name)
    print('target   : %d insns' % len(target))
    print('draft    : %d insns   (%+d)' % (len(draft), len(draft) - len(target)))

    # Byte gate first -- the real criterion.
    if [b for b, _ in draft] == [b for b, _ in target]:
        print('RESULT   : BYTE-EXACT')
        return
    if harness.canonicalise([t for _, t in draft]) == harness.canonicalise([t for _, t in target]):
        print('RESULT   : canonically equal (relocation-only difference)')
        return

    print('RESULT   : differs')
    print()
    print('%-4s %-38s %s' % ('#', 'TARGET', 'DRAFT'))
    n = max(len(draft), len(target))
    shown = 0
    for i in range(n):
        t = target[i][1] if i < len(target) else '--'
        d = draft[i][1] if i < len(draft) else '--'
        if t != d:
            print('%-4d %-38s %s' % (i, t, d))
            shown += 1
            if shown > 40:
                print('... truncated')
                break


main()
