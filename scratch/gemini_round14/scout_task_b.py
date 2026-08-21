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

# Task B region: VA 0x80098350 to 0x8009EAF0
task_b_text = [s for s in sec_symbols.get('.text', []) if 0x80098350 <= s['addr'] < 0x8009EAF0]
print(f"Task B .text symbol count: {len(task_b_text)}")

for i, s in enumerate(task_b_text):
    print(f"[{i:3d}] {hex(s['addr'])} (sz {hex(s['size'])}): {s['name']}")

