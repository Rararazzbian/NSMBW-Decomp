import sys
import os
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
SPLITS_FILE = ROOT / 'bin' / 'dtk' / 'dtk_splits_wiimj2d.txt'
SLICES_JSON = ROOT / 'slices' / 'wiimj2d.json'

# Target range
TGT_START = 0x800272F0
TGT_END = 0x800281C0

print(f"Target range: 0x{TGT_START:08X} - 0x{TGT_END:08X} (span: 0x{TGT_END - TGT_START:X} = {TGT_END - TGT_START} bytes)")

# Find all symbols in this range
syms = []
for line in SYMS_FILE.read_text().splitlines():
    if not line.strip(): continue
    # Format: name = section:0xADDR; // type:... size:0x... scope:...
    m = re.match(r'^(\S+)\s*=\s*(\.\w+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', line.strip())
    if m:
        name, sec, addr_str, meta = m.groups()
        addr = int(addr_str, 16)
        
        # parse meta
        size_m = re.search(r'size:(0x[0-9A-Fa-f]+)', meta)
        size = int(size_m.group(1), 16) if size_m else 0
        type_m = re.search(r'type:(\w+)', meta)
        stype = type_m.group(1) if type_m else ''
        scope_m = re.search(r'scope:(\w+)', meta)
        scope = scope_m.group(1) if scope_m else ''
        
        if sec == '.text' and TGT_START <= addr < TGT_END:
            syms.append({
                'name': name,
                'sec': sec,
                'addr': addr,
                'size': size,
                'type': stype,
                'scope': scope,
                'raw': line
            })

syms.sort(key=lambda s: s['addr'])

print(f"Total .text symbols found: {len(syms)}")
total_code = 0
for i, s in enumerate(syms):
    print(f"{i+1:2d}. 0x{s['addr']:08X} (size 0x{s['size']:03X}) [{s['scope']:6s}] {s['name']}")
    total_code += s['size']

print(f"Total code bytes in symbols: 0x{total_code:X} = {total_code} bytes")
print(f"Padding / gap bytes: {TGT_END - TGT_START - total_code} bytes")
