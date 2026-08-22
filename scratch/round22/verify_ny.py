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
print('=== current file, y-region ===')
i = src.find('s32 ny')
print(src[i-200:i+300])

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=(os.path.join(BASE, 'shadow'),))
print('compile ok:', ok)
harness.disasm(OBJ, DIS)
got = harness.extract(DIS, NAME)
for idx in range(60, 72):
    print('%3d | %s' % (idx, got[idx]))
