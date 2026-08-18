import re
import os
import subprocess

# Let's inspect wiimj2d_symbols.txt
with open('bin/dtk/wiimj2d_symbols.txt', 'r') as f:
    lines = f.readlines()

syms = []
for line in lines:
    line = line.strip()
    if not line:
        continue
    m = re.match(r'^(\S+)\s*=\s*(\.[a-zA-Z0-9_]+):(0x[0-9a-fA-F]+);\s*(?://\s*type:(\S+)\s*size:(0x[0-9a-fA-F]+))?', line)
    if m:
        name, sec, addr_s, stype, size_s = m.groups()
        addr = int(addr_s, 16)
        sz = int(size_s, 16) if size_s else 0
        syms.append({'name': name, 'sec': sec, 'addr': addr, 'type': stype, 'size': sz, 'raw': line})

# Let's check all sections that exist in the symbols
sections = sorted(list(set(s['sec'] for s in syms)))
print("All sections:", sections)

# Search for symbols containing NandThread or mMutex or anything between d_multi_manager and d_next
print("\n=== Symbols matching NandThread or mMutex ===")
for s in syms:
    if 'NandThread' in s['name'] or 'mMutex' in s['name']:
        print(f"{s['sec']}: {hex(s['addr'])} (size {hex(s['size'])}): {s['name']} (type: {s['type']})")

# Let's check sections for d_multi_manager and d_next in dtk_splits_wiimj2d.txt
# Let's parse dtk_splits_wiimj2d.txt
with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r') as f:
    splits_text = f.read()

print("\n=== Splits around d_multi_manager, d_nand_thread, d_next ===")
# Find all entries
entries = splits_text.split('\n\n')
for entry in entries:
    if any(k in entry for k in ['d_multi_manager.cpp', 'd_next.cpp', 'd_mj2d_data.cpp', 'd_rail.cpp']):
        print(entry)
        print("-" * 40)
