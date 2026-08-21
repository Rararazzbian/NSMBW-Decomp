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

# Let us check the pool IDs in .sdata2, .rodata, .data for each candidate region:
# Regions based on sinits:
# 1. d_bc.cpp: 0x8006CF40 - 0x80076BC0
# 2. d_beans_kuribo_mng (or part of d_bc / d_bg): 0x80076BC0 - 0x80076FD0
# 3. d_bg.cpp: 0x80076FD0 - 0x8007E180
# 4. d_bg_actor_mng.cpp: 0x8007E180 - 0x8007F7A0
# 5. dBg_ctr / dBgGlobal / dBgParameter / bgTex / dBgUnit / dBgTexMng / dBlockMng ...
# Let us inspect the sinit of d_bg_unit.cpp (0x80087100) and d_capture_mng.cpp (0x80089ED0)

# Let us inspect all pool IDs in .sdata2:
print("=== sdata2 symbols with addresses ===")
for s in sec_symbols.get('.sdata2', []):
    if 0x8042BF20 <= s['addr'] < 0x8042C2E0:
        print(f"  {hex(s['addr'])}: {s['name']}")

