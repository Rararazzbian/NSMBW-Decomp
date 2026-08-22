import os, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round22'
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

matched, report = harness.diff_fn(TARGET, DIS, 'ProcMain__17dBgActorManager_cFv')
out = os.path.join(BASE, 'procmain_diff_full.txt')
with open(out, 'w') as f:
    f.write(report)

# count diff lines (exclude size header)
lines = report.splitlines()
diff_lines = [l for l in lines if '|' in l and ('want:' in l or 'got:' in l)]
print('Total diff instruction lines:', len(diff_lines))
print('Lines 28+ (beyond the default preview):', max(0, len(diff_lines) - 28))
