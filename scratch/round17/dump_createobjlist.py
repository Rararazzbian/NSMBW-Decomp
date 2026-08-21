import os, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

matched, report = harness.diff_fn(TARGET, DIS, 'createObjList__17dBgActorManager_cFb')
out = os.path.join(BASE, 'createobjlist_diff.txt')
with open(out, 'w') as f:
    f.write(report)
print('Written', len(report), 'bytes, matched:', matched)
