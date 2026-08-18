import os
import re

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

sym_re = re.compile(r'^(\S+)\s*=\s*(\.\w+):0x([0-9a-fA-F]+);\s*(?://\s*(.*))?$')
syms = []
with open(os.path.join(ROOT, 'bin', 'dtk', 'd_en_bossNP_symbols.txt'), 'r', encoding='utf-8') as f:
    for line in f:
        m = sym_re.match(line.strip())
        if m:
            name, sec, addr = m.group(1), m.group(2), int(m.group(3), 16)
            meta = m.group(4) or ''
            sz_m = re.search(r'size:0x([0-9a-fA-F]+)', meta)
            size = int(sz_m.group(1), 16) if sz_m else 0
            syms.append({'name': name, 'sec': sec, 'addr': addr, 'size': size, 'raw': line.strip()})

text_syms = [s for s in syms if s['sec'] == '.text']
text_syms.sort(key=lambda s: s['addr'])

print("Functions between 0x3700 and 0x4600:")
for s in text_syms:
    if 0x3700 <= s['addr'] <= 0x4600:
        print(f"  0x{s['addr']:06x} 0x{s['size']:04x} {s['name']}")
