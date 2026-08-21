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

# Let us check exact boundaries and symbol stats for the 5 candidate TUs:
# Candidate 1: d_bc.cpp
# Candidate 2: d_bg.cpp
# Candidate 3: d_bg_actor_mng.cpp
# Candidate 4: d_bg_unit.cpp
# Candidate 5: d_capture_mng.cpp

# Let's inspect .text ranges for each:
# 1. d_bc.cpp: 0x8006CF40 to 0x80076BC0
#    Wait, does d_bc end at 0x80076BC0 or 0x80076FD0 (including dBeansKuriboMng)?
# 2. d_bg.cpp: starts at 0x80076FD0 (or 0x80076BC0), ends at 0x8007E180
# 3. d_bg_actor_mng.cpp: starts at 0x8007E180, ends at 0x8007F7A0
# 4. d_bg_unit.cpp: starts at 0x8007F7A0, ends at 0x800872E0 (or 0x80089C90?)
# 5. d_capture_mng.cpp: starts at 0x800872E0 or 0x80089C90, ends at 0x8008C200

# Let's check the boundary between d_bg_unit and d_capture_mng!
# Where does d_bg_unit end and d_capture_mng start?
# Let's check text around 0x80087100 (__sinit_\d_bg_unit_cpp) and 0x80089ed0 (__sinit_\d_capture_mng_cpp)
print("=== Functions around d_bg_unit and d_capture_mng ===")
text_syms = [s for s in sec_symbols.get('.text', []) if 0x80086F00 <= s['addr'] <= 0x8008A200]
for s in text_syms:
    print(f"  {hex(s['addr'])}: {s['name']}")

