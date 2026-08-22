import os
import re
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

# y-expression variants to test (each replaces the pos.y line)
Y_OLD = '        pos.y = (f32)((int)((-(s32)m_pObjList[i].mY + y0) << 4));\n'
VARIANTS = {
    'helper_neg': (
        'static inline s32 round22_neg(s32 v) { return -v; }\n',
        '        pos.y = (f32)((int)((y0 + round22_neg((s32)m_pObjList[i].mY)) << 4));\n',
    ),
    'xor_neg': (
        '',
        '        pos.y = (f32)((int)((y0 + (m_pObjList[i].mY ^ 0xFFFF) + 1) << 4));\n',
    ),
    'neg_local_assign': (
        '',
        '        s32 ny = m_pObjList[i].mY;\n        ny = -ny;\n        pos.y = (f32)((int)((y0 + ny) << 4));\n',
    ),
    'plain_sub': (
        '',
        '        pos.y = (f32)((int)((y0 - m_pObjList[i].mY) << 4));\n',
    ),
}

want = harness.extract(TARGET, NAME)

for label, (helper, yline) in VARIANTS.items():
    new_src = src.replace(Y_OLD, yline)
    if helper:
        # insert helper after the includes / before first method
        anchor = 'dBgActorManager_c *dBgActorManager_c::ms_instance;'
        new_src = new_src.replace(anchor, helper + anchor)
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
    has_neg = any('neg ' in l for l in got[60:70])
    has_subf = any('subf' in l for l in got[60:70])
    print('%s: len %d/%d diff %d neg-in-y-region=%s subf-in-y-region=%s' %
          (label, len(got), len(want), diff, has_neg, has_subf))
