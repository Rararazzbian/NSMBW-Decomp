import json
import re

with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)

sec_meta = slice_data['meta']['sections']
sec_bases = {k: int(v['addr'], 16) for k, v in sec_meta.items()}

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

# Let's inspect each section between d_base_actor and d_cc:
# Slices in json:
# d_base_actor:
#   text: 0x8006c6d0 - 0x8006cf40
#   ctors: 0x802edce4 + 0xa8 (0x802edd8c)
#   data: 0x8030f528 - 0x8030f588
#   sbss: 0x8042a078 - 0x8042a088
#   sdata2: 0x8042bf10 - 0x8042bf20
# d_cc:
#   text: 0x8008c200 - 0x8008dc80
#   ctors: 0x802edce4 + 0xc0 (0x802edda4)
#   rodata: 0x802f03e8 - 0x802f03f8
#   data: 0x803110a0 - 0x80311100
#   sbss: 0x8042a140 - 0x8042a150
#   sdata2: 0x8042c2e0 - 0x8042c300

# Let's check if there are any .bss symbols between d_base_actor (slice 37 has no bss) and d_cc (slice 38 has no bss)
# Let's check which slice before/after has bss:
# Slice 33 d_actor.cpp: bss 0x80351980 + 0x4898 = 0x80356218?
# Let's check all bss symbols in wiimj2d_symbols.txt around 0x80356200
print("=== All BSS symbols around 0x80356000 - 0x80358000 ===")
for s in sec_symbols.get('.bss', []):
    if 0x80355000 <= s['addr'] <= 0x80358000:
        print(f"  {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

print("=== All SDATA symbols around 0x80427980 ===")
for s in sec_symbols.get('.sdata', []):
    if 'dBc' in s['name'] or 'dBg' in s['name'] or 'Capture' in s['name'] or 'Beans' in s['name']:
        print(f"  {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

