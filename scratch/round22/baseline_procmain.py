import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
SRC = os.path.join(BASE, 'd_bg_actor_mng.cpp')
OBJ = os.path.join(BASE, 'd_bg_actor_mng.o')
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=(os.path.join(BASE, 'shadow'),))
if not ok:
    print('COMPILE FAILED')
    sys.exit(1)
ok2, log2 = harness.disasm(OBJ, DIS)
if not ok2:
    print('DISASM FAILED')
    sys.exit(1)

NAME = 'ProcMain__17dBgActorManager_cFv'
want = harness.extract(TARGET, NAME)
got = harness.extract(DIS, NAME)
print('target %d, draft %d' % (len(want), len(got)))
diff = 0
for i in range(max(len(want), len(got))):
    a = want[i] if i < len(want) else '<none>'
    b = got[i] if i < len(got) else '<none>'
    if a != b:
        diff += 1
print('TOTAL DIFF (round17 baseline):', diff)
