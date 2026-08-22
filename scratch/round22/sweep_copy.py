import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round22')
SRC = os.path.join(BASE, 'd_bg_actor_mng.cpp')
OBJ = os.path.join(BASE, 'd_bg_actor_mng.o')
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')
NAME = 'ProcMain__17dBgActorManager_cFv'

with open(SRC, 'r', encoding='utf-8') as f:
    src = f.read()

OLD = '''    mVec3_c viewMin = mMin;
    mVec3_c viewMax = mMax;
'''
VARIANTS = {
    'memcpy': '''    mVec3_c viewMin;
    mVec3_c viewMax;
    memcpy(&viewMin, &mMin, sizeof(mVec3_c));
    memcpy(&viewMax, &mMax, sizeof(mVec3_c));
''',
    'podcast': '''    struct Vec3POD {
        f32 x, y, z;
    };
    Vec3POD viewMinP = *(Vec3POD *)&mMin;
    Vec3POD viewMaxP = *(Vec3POD *)&mMax;
    mVec3_c &viewMin = *(mVec3_c *)&viewMinP;
    mVec3_c &viewMax = *(mVec3_c *)&viewMaxP;
''',
}

want = harness.extract(TARGET, NAME)
for label, new in VARIANTS.items():
    new_src = src.replace(OLD, new)
    tmp = os.path.join(BASE, 'variant_tmp.cpp')
    with open(tmp, 'w', encoding='utf-8') as f:
        f.write(new_src)
    ok, log = harness.compile_draft(tmp, OBJ, extra_inc=(os.path.join(BASE, 'shadow'),))
    if not ok:
        print('%s: COMPILE FAILED' % label)
        continue
    harness.disasm(OBJ, DIS)
    got = harness.extract(DIS, NAME)
    diff = sum(1 for i in range(max(len(want), len(got)))
               if (want[i] if i < len(want) else '<none>') != (got[i] if i < len(got) else '<none>'))
    # count lwz/stw vs lfs/stfs in preamble (indices 15..45)
    ints = sum(1 for l in got[15:46] if l.startswith('lwz ') or l.startswith('stw '))
    flts = sum(1 for l in got[15:46] if l.startswith('lfs ') or l.startswith('stfs '))
    print('%s: len %d/%d diff %d | preamble int-copy instrs=%d float-copy instrs=%d' %
          (label, len(got), len(want), diff, ints, flts))
    if diff < 45:
        for i in range(max(len(want), len(got))):
            a = want[i] if i < len(want) else '<none>'
            b = got[i] if i < len(got) else '<none>'
            if a != b:
                print('  %3d | want: %-48s got: %s' % (i, a, b))
