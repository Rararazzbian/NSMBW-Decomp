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

text_syms = [s for s in sec_symbols.get('.text', []) if 0x8006CF40 <= s['addr'] < 0x8008C200]
for i in range(len(text_syms)):
    curr = text_syms[i]
    if i + 1 < len(text_syms):
        curr['size'] = text_syms[i+1]['addr'] - curr['addr']
    else:
        curr['size'] = 0x8008C200 - curr['addr']

# Let's inspect the boundaries between:
# 1. d_bc.cpp: 0x8006cf40 to ... ?
# Where does d_bc end and d_bg begin?
# Let's check text around 0x80076bb0 (__sinit_\d_bc_cpp) and 0x80076fd0 (bg_createHeap__5dBg_cFv)
print("=== Text around d_bc / d_bg boundary ===")
for s in text_syms:
    if 0x80076b00 <= s['addr'] <= 0x80077100:
        print(f"  {hex(s['addr'])} (+{hex(s['size'])}): {s['name']}")

# Let's check vtable of dBc_c and dBg_c
# In .data:
# 0x8030f6d0: __vt__5dBc_c
# 0x8030f790: __vt__5dBg_c

print("=== Data symbols around dBc_c / dBg_c ===")
for s in sec_symbols.get('.data', []):
    if 0x8030F588 <= s['addr'] <= 0x80310000:
        print(f"  {hex(s['addr'])}: {s['name']}")

