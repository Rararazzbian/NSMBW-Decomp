"""Does the Gap A shape replace the `volatile` hack in fn_800C31C0?

Gap A's finding, stated as a rule: the field loads MWCC emits for an AGGREGATE
COPY (`local = obj;`) are not common-subexpression-eliminated against earlier
SCALAR reads of the same members. That is a forced reload with no `volatile`
anywhere -- exactly what fn_800C31C0 needs.

Control here is the HONEST source (no volatile), which is 547/549.

Usage:  python try3.py <variant>  |  python try3.py --list
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(HERE, 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
FN = 'fn_800C31C0'
DRAFT_FN = 'fn_800C31C0__FP10dLineMng_c'

SZ = 'dLineMng_c::smc_UNIT_SIZE_X'
ANCHOR = '''    mVec2_c base;
    base.x = %s * (f32)(int)(*(volatile f32 *)&self->mPos.x / %s) - 16.0f;
    base.y = %s * (f32)(int)(*(volatile f32 *)&self->mPos.y / %s) - 16.0f;''' % (SZ, SZ, SZ, SZ)

VARIANTS = {
    # Honest baseline: plain reads, no volatile. Known 547/549.
    'control': '''    mVec2_c base;
    base.x = %s * (f32)(int)(self->mPos.x / %s) - 16.0f;
    base.y = %s * (f32)(int)(self->mPos.y / %s) - 16.0f;''' % (SZ, SZ, SZ, SZ),

    # The volatile hack, for reference. Known 549/549, not shippable.
    'volatile': ANCHOR,

    # G1: aggregate copy, then rewrite each field in place. The proven Gap A
    #     shape, applied to the object rather than to one field.
    'copy_self': '''    mVec2_c base = self->mPos;
    base.x = %s * (f32)(int)(base.x / %s) - 16.0f;
    base.y = %s * (f32)(int)(base.y / %s) - 16.0f;''' % (SZ, SZ, SZ, SZ),

    # G2: copy into a separate local, read the copy. Keeps `base` uninitialised
    #     at declaration, in case the copy-init is what MWCC folds.
    'copy_tmp': '''    mVec2_c snap = self->mPos;
    mVec2_c base;
    base.x = %s * (f32)(int)(snap.x / %s) - 16.0f;
    base.y = %s * (f32)(int)(snap.y / %s) - 16.0f;''' % (SZ, SZ, SZ, SZ),
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

    src = open(BASE, encoding='utf-8').read()
    if src.count(ANCHOR) != 1:
        sys.exit('ANCHOR NOT FOUND/UNIQUE: %d' % src.count(ANCHOR))
    out_src = os.path.join(HERE, 'g_%s.cpp' % name)
    open(out_src, 'w', encoding='utf-8').write(src.replace(ANCHOR, VARIANTS[name]))

    obj = os.path.join(HERE, 'g_%s.o' % name)
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[INC])
    if not ok:
        sys.exit('COMPILE FAILED\n' + log[-3000:])
    txt = os.path.join(HERE, 'g_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        sys.exit('DISASM FAILED\n' + log[-2000:])

    fns = parse(txt)
    # The static may be emitted under a mangled/local name; find it by size.
    draft = fns.get(DRAFT_FN)
    if draft is None:
        cands = [k for k in fns if 'C31C0' in k or k.startswith('fn_')]
        draft = fns[cands[0]] if cands else None
    target = parse(TARGET).get(FN)
    if draft is None:
        sys.exit('draft function not found. names: %s' % list(fns)[:8])
    if target is None:
        sys.exit('target function not found')

    print('variant  : %-12s target %d   draft %d   (%+d)'
          % (name, len(target), len(draft), len(draft) - len(target)))


main()
