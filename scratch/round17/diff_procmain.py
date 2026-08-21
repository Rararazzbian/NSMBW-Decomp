import os, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

matched, report = harness.diff_fn(TARGET, DIS, 'ProcMain__17dBgActorManager_cFv')
with open(os.path.join(BASE, 'diff_procmain.txt'), 'w') as f:
    f.write(report)
print('Wrote diff_procmain.txt, matched:', matched)
