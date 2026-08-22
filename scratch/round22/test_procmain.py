import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round22')
SRC = os.path.join(BASE, 'd_bg_actor_mng.cpp')
OBJ = os.path.join(BASE, 'd_bg_actor_mng.o')
DISASM = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=(os.path.join(BASE, 'shadow'),))
if not ok:
    print('COMPILE FAILED')
    print(log)
    sys.exit(1)

ok2, log2 = harness.disasm(OBJ, DISASM)
if not ok2:
    print('DISASM FAILED', log2)
    sys.exit(1)

matched, report = harness.diff_fn(TARGET, DISASM, 'ProcMain__17dBgActorManager_cFv')
lines = report.splitlines()
print('MATCH' if matched else 'DIFFER')
for l in lines[:3]:
    print('  ' + l)
diff_lines = [l for l in lines if '|' in l and ('want:' in l or 'got:' in l)]
print('Total diff instruction lines:', len(diff_lines))
