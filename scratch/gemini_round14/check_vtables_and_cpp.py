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

# Find any symbol containing @unnamed@ or _cpp in Task A
print("=== Symbols containing _cpp in Task A ===")
for s in symbols:
    if 0x8006CF40 <= s['addr'] < 0x8008C200 or ('_cpp' in s['name'] and any(k in s['name'] for k in ['bc', 'bg', 'capture', 'beans'])):
        if '_cpp' in s['name']:
            print(f"  {s['sec']} {hex(s['addr'])}: {s['name']}")

print("\n=== All vtables in Task A range (.data: 0x8030F588 to 0x803110A0) ===")
for s in symbols:
    if s['sec'] == '.data' and 0x8030F588 <= s['addr'] < 0x803110A0:
        if '__vt__' in s['name']:
            print(f"  {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

