"""Probe the symbol map and slice file for the d_a_wm_ghost range."""
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))

LO = 0x163620
HI = 0x164230

# 1) symbols in range
pat = re.compile(r'^(\S+) = \.(\w+):(0x[0-9A-Fa-f]+); // type:(\w+) size:(0x[0-9A-Fa-f]+) scope:(\w+)')
in_range = []
with open(os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt'), encoding='utf-8', errors='replace') as fh:
    for line in fh:
        m = pat.match(line.strip())
        if not m:
            continue
        name, sec, addr, typ, size, scope = m.groups()
        if sec == 'text' and LO <= int(addr, 16) < HI:
            in_range.append((int(addr, 16), int(size, 16), name, scope))

in_range.sort()
print('=== .text symbols in [0x%06X, 0x%06X) ===' % (LO, HI))
total = 0
for addr, size, name, scope in in_range:
    print('0x%06X  size 0x%-4X  %-40s %s' % (addr, size, name, scope))
    total += size
print('count=%d  total code size=0x%X (%d bytes)' % (len(in_range), total, total))
print('span size = 0x%X (%d bytes)' % (HI - LO, HI - LO))

# 2) context: neighbours just outside the range
print()
print('=== neighbours (2 before / 2 after in file order) ===')
all_text = []
with open(os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt'), encoding='utf-8', errors='replace') as fh:
    for line in fh:
        m = pat.match(line.strip())
        if not m:
            continue
        name, sec, addr, typ, size, scope = m.groups()
        if sec == 'text':
            all_text.append((int(addr, 16), int(size, 16), name))
all_text.sort()
idx = [i for i, (a, s, n) in enumerate(all_text) if LO <= a < HI]
if idx:
    for i in idx[0] - 2:
        if 0 <= i < len(all_text):
            a, s, n = all_text[i]
            print('BEFORE 0x%06X size 0x%-4X %s' % (a, s, n))
    for i in idx[-1] + 1:
        if i < len(all_text):
            a, s, n = all_text[i]
            print('AFTER  0x%06X size 0x%-4X %s' % (a, s, n))
            break
    for i in range(idx[-1] + 1, min(idx[-1] + 3, len(all_text))):
        a, s, n = all_text[i]
        print('AFTER  0x%06X size 0x%-4X %s' % (a, s, n))

# 3) slice file meta (section bases) and our claimed slices
print()
with open(os.path.join(ROOT, 'slices', 'd_basesNP.json'), encoding='utf-8') as fh:
    sl = json.load(fh)
print('meta.sections:', json.dumps(sl.get('meta', {}).get('sections', {}), indent=1))
print('defaultCompilerFlags:', sl.get('meta', {}).get('defaultCompilerFlags'))
print()
print('=== slices intersecting our claimed ranges ===')
claims = {
    'text': (0x163620, 0x164230),
    'ctors': (0x3E0, 0x3E4),
    'rodata': (0x8880, 0x88B8),
    'data': (0x44A9C, 0x44CB4),
    'bss': (0xFDC0, 0xFDD0),
}
for sec, (lo, hi) in claims.items():
    hits = []
    for s in sl.get('slices', []):
        a = s.get('start')
        b = s.get('end')
        if a is None or b is None:
            continue
        a, b = int(a), int(b)
        if sec in (s.get('section', '')) and not (b <= lo or a >= hi):
            hits.append((a, b, s))
    print('--- %s [%06X, %06X)' % (sec, lo, hi))
    for a, b, s in hits:
        print('    %06X-%06X  %s' % (a, b, json.dumps({k: v for k, v in s.items() if k in ('source', 'unit', 'file', 'name')})))
