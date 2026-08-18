import os
import sys
import json
import re

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

# Load d_basesNP_symbols.txt
syms = []
with open(os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt')) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#') or line.startswith('//'): continue
        m = re.match(r'^(\S+)\s*=\s*(\.\w+):0x([0-9a-fA-F]+);\s*(?://\s*(.*))?$', line)
        if m:
            name, sec, addr_str, meta = m.group(1), m.group(2), m.group(3), m.group(4) or ''
            addr = int(addr_str, 16)
            size = 0
            sz_m = re.search(r'size:0x([0-9a-fA-F]+)', meta)
            if sz_m: size = int(sz_m.group(1), 16)
            syms.append({'name': name, 'sec': sec, 'addr': addr, 'size': size, 'meta': meta})

def get_syms_in(sec, start, end):
    return [s for s in syms if s['sec'] == sec and start <= s['addr'] < end]

print("=== GRID ANALYSIS ===")
# Let's check grid around:
# .text: 0x164210 - 0x164404
# .ctors: 0x3e4 - 0x3e8
# .rodata: 0x88b8 - 0x88c8
# .data: 0x44c90 - 0x44d20
# .bss: 0xfdd0 - 0xfde0
for sec, (s, e) in [
    ('.text', (0x164200, 0x164420)),
    ('.ctors', (0x3e0, 0x3ec)),
    ('.rodata', (0x88a0, 0x88d0)),
    ('.data', (0x44c80, 0x44d30)),
    ('.bss', (0xfdc0, 0xfdf0)),
]:
    print(f"\n--- {sec} in [0x{s:x}, 0x{e:x}) ---")
    for sym in get_syms_in(sec, s, e):
        print(f"  0x{sym['addr']:06x} size:0x{sym['size']:04x} {sym['name']}  ({sym['meta']})")

print("\n=== TOWER ANALYSIS ===")
# Let's check tower around:
# .text: 0x1856f0 - 0x185b44
# .ctors: 0x44c - 0x450
# .rodata: 0x9320 - 0x9330
# .data: 0x48090 - 0x48158
# .bss: 0x10350 - 0x10360
for sec, (s, e) in [
    ('.text', (0x1856e0, 0x185b60)),
    ('.ctors', (0x448, 0x454)),
    ('.rodata', (0x9310, 0x9340)),
    ('.data', (0x48080, 0x48170)),
    ('.bss', (0x10340, 0x10370)),
]:
    print(f"\n--- {sec} in [0x{s:x}, 0x{e:x}) ---")
    for sym in get_syms_in(sec, s, e):
        print(f"  0x{sym['addr']:06x} size:0x{sym['size']:04x} {sym['name']}  ({sym['meta']})")
