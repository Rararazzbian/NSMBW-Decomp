import os, re, sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

LO, HI = 0x163620, 0x164230
objdir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')
starts = []
for fn in sorted(os.listdir(objdir)):
    m = re.match(r'auto_00_([0-9A-Fa-f]{6})_text\.o$', fn)
    if m:
        starts.append((int(m.group(1), 16), fn))
starts.sort()
# find the object(s) that start before LO (candidates that straddle)
below = [s for s in starts if s[0] < LO]
# the straddling object is the largest start below LO (objects are contiguous splits)
cand = below[-3:]
print('objects starting just before 0x%X:' % LO)
for a, fn in cand:
    print('  0x%06X  %s' % (a, fn))
