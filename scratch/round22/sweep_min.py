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

OLD = '''        pos.x += m_pObjList[i].getOffset().x;
        pos.y += m_pObjList[i].getOffset().y;
        pos.z = 0.0f;
        mVec3_c mMin = pos;
'''
VARIANTS = {
    'plain_y_add': '''        pos.x += m_pObjList[i].getOffset().x;
        pos.y = pos.y + m_pObjList[i].getOffset().y;
        pos.z = 0.0f;
        mVec3_c mMin = pos;
''',
    'preload_px': '''        f32 px = pos.x;
        pos.x += m_pObjList[i].getOffset().x;
        pos.y += m_pObjList[i].getOffset().y;
        pos.z = 0.0f;
        mVec3_c mMin = pos;
        mMin.x = px;
''',
    'y_then_x_offset': '''        pos.y += m_pObjList[i].getOffset().y;
        pos.x += m_pObjList[i].getOffset().x;
        pos.z = 0.0f;
        mVec3_c mMin = pos;
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
        print(log[:400])
        continue
    harness.disasm(OBJ, DIS)
    got = harness.extract(DIS, NAME)
    diff = sum(1 for i in range(max(len(want), len(got)))
               if (want[i] if i < len(want) else '<none>') != (got[i] if i < len(got) else '<none>'))
    print('%s: len %d/%d diff %d' % (label, len(got), len(want), diff))
    if diff < 45:
        for i in range(78, 100):
            a = want[i] if i < len(want) else '<none>'
            b = got[i] if i < len(got) else '<none>'
            if a != b:
                print('  %3d | want: %-48s got: %s' % (i, a, b))
