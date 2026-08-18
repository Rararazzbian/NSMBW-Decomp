import os, re

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
objdir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')
starts = []
for fn in sorted(os.listdir(objdir)):
    m = re.match(r'auto_00_([0-9A-Fa-f]+)_text\.o$', fn)
    if m:
        starts.append((int(m.group(1), 16), fn))
starts.sort()
print('total text objects:', len(starts))
print('min:', hex(starts[0][0]), starts[0][1])
print('max:', hex(starts[-1][0]), starts[-1][1])
near = [s for s in starts if 0x150000 <= s[0] <= 0x170000]
for a, fn in near:
    print('  0x%06X  %s' % (a, fn))
