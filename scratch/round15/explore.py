import os, re, sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

LO, HI = 0x163620, 0x164230
symfile = os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt')

# 1. parse the symbol map for .text functions in range
fns = []
with open(symfile, encoding='utf-8', errors='replace') as f:
    for line in f:
        m = re.match(r'^(fn_2_\S+|_prolog|_epilog) = \.text:(0x[0-9A-Fa-f]+); // type:function size:(0x[0-9A-Fa-f]+)', line)
        if not m:
            continue
        name, addr, size = m.group(1), int(m.group(2), 16), int(m.group(3), 16)
        if LO <= addr < HI:
            fns.append((addr, size, name))
fns.sort()
total = sum(s for _, s, _ in fns)
print('functions in [0x%X, 0x%X): %d, code bytes: %d (0x%X)' % (LO, HI, len(fns), total, total))
for addr, size, name in fns:
    print('  0x%06X  size 0x%4X  %s' % (addr, size, name))
# gaps
prev = LO
for addr, size, name in fns:
    if addr > prev:
        print('  GAP 0x%X-0x%X (%d bytes) before %s' % (prev, addr, addr - prev, name))
    prev = addr + size
if prev < HI:
    print('  TRAILING 0x%X-0x%X (%d bytes)' % (prev, HI, HI - prev))

# 2. find the auto_* objects whose start addr is within [LO-0x1000, HI]
objdir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')
cands = []
for fn in sorted(os.listdir(objdir)):
    m = re.match(r'auto_00_([0-9A-Fa-f]{6})_text\.o$', fn)
    if m:
        a = int(m.group(1), 16)
        if LO - 0x800 <= a < HI:
            cands.append((a, fn))
cands.sort()
print('\nobjects starting in [0x%X, 0x%X):' % (LO - 0x800, HI))
for a, fn in cands:
    print('  0x%06X  %s' % (a, fn))
