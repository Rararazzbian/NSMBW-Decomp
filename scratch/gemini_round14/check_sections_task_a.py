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

# Let's inspect all symbols across all sections between d_base_actor and d_cc:
# d_base_actor:
#   .text: 0x65f50-0x667c0 -> 0x8006c6d0 - 0x8006cf40
#   .data: 0x10e88-0x10ee8 -> 0x802fe6a0 + 0x10e88 = 0x8030f528 - 0x8030f588
#   .sbss: 0x1d8-0x1e8     -> 0x80429ea0 + 0x1d8 = 0x8042a078 - 0x8042a088
#   .sdata2: 0xbb0-0xbc0   -> 0x8042b360 + 0xbb0 = 0x8042bf10 - 0x8042bf20
# d_cc:
#   .text: 0x85a80-0x87500 -> 0x80006780 + 0x85a80 = 0x8008c200 - 0x8008dc80
#   .rodata: 0x2408-0x2418 -> 0x802edfe0 + 0x2408 = 0x802f03e8 - 0x802f03f8
#   .data: 0x12a00-0x12a60 -> 0x802fe6a0 + 0x12a00 = 0x803110a0 - 0x80311100
#   .sbss: 0x2a0-0x2b0     -> 0x80429ea0 + 0x2a0 = 0x8042a140 - 0x8042a150
#   .sdata2: 0xf80-0xfa0   -> 0x8042b360 + 0xf80 = 0x8042c2e0 - 0x8042c300

print("=== DATA symbols in range (0x8030F588 to 0x803110A0) ===")
for s in sec_symbols.get('.data', []):
    if 0x8030F588 <= s['addr'] < 0x803110A0:
        print(f"  {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

print("\n=== RODATA symbols in range (0x802EDFE0 to 0x802F03E8) ===")
for s in sec_symbols.get('.rodata', []):
    if 0x802EDFE0 <= s['addr'] < 0x802F03E8:
        print(f"  {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

print("\n=== BSS symbols in range ===")
# Let's find BSS bounds around this
for s in sec_symbols.get('.bss', []):
    if 'dBg' in s['name'] or 'dBc' in s['name'] or 'Capture' in s['name'] or 'Beans' in s['name']:
        print(f"  {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

print("\n=== SBSS symbols in range (0x8042A088 to 0x8042A140) ===")
for s in sec_symbols.get('.sbss', []):
    if 0x8042A088 <= s['addr'] < 0x8042A140:
        print(f"  {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

print("\n=== SDATA2 symbols in range (0x8042BF20 to 0x8042C2E0) ===")
for s in sec_symbols.get('.sdata2', []):
    if 0x8042BF20 <= s['addr'] < 0x8042C2E0:
        print(f"  {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

