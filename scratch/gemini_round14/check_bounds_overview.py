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

# Let us list every symbol in each section for Task A range
# Task A text range: 0x8006CF40 to 0x8008C200
# Slices in json:
# Slice 37 d_base_actor.cpp:
#   text: 0x65f50-0x667c0 (VA 0x8006c6d0 - 0x8006cf40)
#   ctors: 0xa8-0xac
#   data: 0x10e88-0x10ee8 (VA 0x8030f528 - 0x8030f588)
#   sbss: 0x1d8-0x1e8 (VA 0x8042a078 - 0x8042a088)
#   sdata2: 0xbb0-0xbc0 (VA 0x8042bf10 - 0x8042bf20)
# Slice 38 d_cc.cpp:
#   text: 0x85a80-0x87500 (VA 0x8008c200 - 0x8008dc80)
#   ctors: 0xc0-c4 (0x802edce4 + 0xc0 = 0x802edda4)
#   rodata: 0x2408-0x2418 (VA 0x802f03e8 - 0x802f03f8)
#   data: 0x12a00-0x12a60 (VA 0x803110a0 - 0x80311100)
#   sbss: 0x2a0-0x2b0 (VA 0x8042a140 - 0x8042a150)
#   sdata2: 0xf80-0xfa0 (VA 0x8042c2e0 - 0x8042c300)

print("=== Task A Section Bounds ===")
print("text:   0x8006CF40 - 0x8008C200 (size: 0x1F2C0 = 127,680 bytes)")
print("ctors:  0x802EDCE4 + 0xAC to 0xC0 (5 slots: 0x802EDD90 to 0x802EDDA4)")
print("rodata: 0x802EDFE0 - 0x802F03E8 (size: 0x2408 = 9,224 bytes)")
print("data:   0x8030F588 - 0x803110A0 (size: 0x1B18 = 6,936 bytes)")
print("sbss:   0x8042A088 - 0x8042A140 (size: 0xB8 = 184 bytes)")
print("sdata2: 0x8042BF20 - 0x8042C2E0 (size: 0x3C0 = 960 bytes)")

