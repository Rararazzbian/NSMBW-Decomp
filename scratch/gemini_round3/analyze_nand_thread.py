import re
import os

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
        syms.append({'name': name, 'sec': sec, 'addr': addr, 'type': stype, 'size': sz, 'line': line})

print(f"Total parsed symbols: {len(syms)}")

text_syms = [s for s in syms if s['sec'] == '.text' and 0x800CED00 <= s['addr'] < 0x800CFCE0]
print(f"Text syms in range (0x800CED00 - 0x800CFCE0): {len(text_syms)}")
for i, s in enumerate(text_syms):
    print(f"{i+1:2d}. {hex(s['addr'])} (size {hex(s['size'])} = {s['size']:4d} B): {s['name']}")

# Check total size
total_text = sum(s['size'] for s in text_syms)
print(f"Total .text size in symbols: {total_text} bytes (0x{total_text:X}), range size: {0x800CFCE0 - 0x800CED00} bytes")
