import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round22')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')
DIS = os.path.join(BASE, 'draft_disasm.txt')
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
        print('%3d | want: %-48s got: %s' % (i, a, b))
print('TOTAL DIFF:', diff)
