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

# Calculate function sizes
for i in range(len(text_syms)):
    curr = text_syms[i]
    if i + 1 < len(text_syms):
        curr['size'] = text_syms[i+1]['addr'] - curr['addr']
    else:
        curr['size'] = 0x8008C200 - curr['addr']

# Let's inspect all classes/namespaces/prefixes in text_syms
print(f"Total symbols in text: {len(text_syms)}")

# Let's see all unique classes in Task A text
classes = set()
for s in text_syms:
    name = s['name']
    # Demangle simple patterns: __XXClassName_c or method__ClassName_c
    m = re.search(r'__(\d+)([a-zA-Z0-9_]+)', name)
    if m:
        cname = m.group(2)[:int(m.group(1))]
        classes.add(cname)
    elif 'fn_' in name:
        classes.add('ANON')

print("Classes found in Task A text:")
for c in sorted(classes):
    print(f"  {c}")

