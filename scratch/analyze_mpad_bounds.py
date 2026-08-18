import sys
import os
import json
import re

# Load wiimj2d_symbols.txt
symbols = []
with open('bin/dtk/wiimj2d_symbols.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        # name = .sec:0xaddr; // type:... size:0x...
        m = re.match(r'^([^=]+)=\s*([^:]+):(0x[0-9a-fA-F]+);\s*//\s*(.*)$', line)
        if m:
            name, sec, addr, comment = m.groups()
            size_m = re.search(r'size:(0x[0-9a-fA-F]+)', comment)
            size = int(size_m.group(1), 16) if size_m else 0
            symbols.append({
                'name': name.strip(),
                'sec': sec.strip(),
                'addr': int(addr, 16),
                'size': size,
                'comment': comment
            })

# Find all symbols related to mPad / m_pad_cpp
m_pad_syms = [s for s in symbols if 'mPad' in s['name'] or 'm_pad' in s['name']]
print("m_pad symbols in wiimj2d_symbols.txt:")
for s in m_pad_syms:
    print(f"  {s['sec']}: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

