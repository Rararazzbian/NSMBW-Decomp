import re
import os

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

with open(os.path.join(ROOT, 'bin', 'dtk', 'd_enemiesNP_symbols.txt')) as f:
    sym_lines = f.readlines()

data_syms = []
for l in sym_lines:
    if '.data:' in l:
        m = re.match(r'^(.*?)\s*=\s*\.data:(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', l.strip())
        if m:
            name, addr_s, rest = m.groups()
            sz_m = re.search(r'size:(0x[0-9A-Fa-f]+|\d+)', rest)
            sz = int(sz_m.group(1), 16 if sz_m.group(1).startswith('0x') else 10) if sz_m else 0
            addr = int(addr_s, 16)
            data_syms.append((addr, sz, name, rest))

data_syms.sort()

# Print symbols around block_cloud (0x5e00 .. 0x6300)
print('=== .data around block_cloud ===')
for a, sz, n, r in data_syms:
    if 0x5DF0 <= a <= 0x6290:
        print(f'{hex(a)} (sz={hex(sz)}): {n}')
