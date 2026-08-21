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

text_syms = [s for s in sec_symbols.get('.text', []) if 0x8006CF40 <= s['addr'] < 0x8008C200]

for i in range(len(text_syms)):
    curr = text_syms[i]
    if i + 1 < len(text_syms):
        curr['inferred_size'] = text_syms[i+1]['addr'] - curr['addr']
    else:
        curr['inferred_size'] = 0x8008C200 - curr['addr']

with open('scratch/gemini_round14/task_a_funcs.txt', 'w', encoding='utf-8') as f:
    for i, s in enumerate(text_syms):
        name = s['name']
        addr = s['addr']
        sz = s['inferred_size']
        is_anon = name.startswith('fn_') or name.startswith('lbl_')
        f.write(f"[{i:3d}] {hex(addr)} (+0x{sz:4x}) {'[ANON]' if is_anon else '[NAME]':<7} {name}\n")

print(f"Wrote {len(text_syms)} functions to task_a_funcs.txt")

