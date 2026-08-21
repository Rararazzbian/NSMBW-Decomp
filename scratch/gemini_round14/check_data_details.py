import json
import re

symbols = []
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('//') or '=' not in line:
            continue
        parts = line.split('=', 1)
        sym_name = parts[0].strip()
        rest = parts[1].strip()
        m = re.match(r'(\.[a-zA-Z0-9_\$]+):0x([0-9a-fA-F]+);\s*(//\s*size:0x([0-9a-fA-F]+))?', rest)
        if m:
            sec = m.group(1)
            addr = int(m.group(2), 16)
            size = int(m.group(4), 16) if m.group(4) else 0
            symbols.append({'name': sym_name, 'sec': sec, 'addr': addr, 'size': size, 'raw': line})

sec_symbols = {}
for s in symbols:
    sec_symbols.setdefault(s['sec'], []).append(s)
for sec in sec_symbols:
    sec_symbols[sec].sort(key=lambda x: (x['addr'], x['size']))

# Let us list every object in .data from 0x8030F588 to 0x803110A0 with exact sizes
data_syms = [s for s in sec_symbols.get('.data', []) if 0x8030F588 <= s['addr'] < 0x803110A0]
for i in range(len(data_syms)):
    curr = data_syms[i]
    if i + 1 < len(data_syms):
        curr['size'] = data_syms[i+1]['addr'] - curr['addr']
    else:
        curr['size'] = 0x803110A0 - curr['addr']

print("=== Detailed .data symbols in Task A ===")
for s in data_syms:
    print(f"  {hex(s['addr'])} (+0x{s['size']:04x}): {s['name']}")

