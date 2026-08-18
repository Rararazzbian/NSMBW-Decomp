import sys
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
SPLITS_FILE = ROOT / 'bin' / 'dtk' / 'dtk_splits_wiimj2d.txt'

lines = SYMS_FILE.read_text().splitlines()

# Let's inspect .text symbols from 0x800272F0 to 0x8002AB40
text_syms = []
for line in lines:
    m = re.match(r'^(\S+)\s*=\s*\.text:(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', line.strip())
    if m:
        name, addr_str, meta = m.groups()
        addr = int(addr_str, 16)
        if 0x800272F0 <= addr < 0x8002AB40:
            size_m = re.search(r'size:(0x[0-9A-Fa-f]+)', meta)
            size = int(size_m.group(1), 16) if size_m else 0
            text_syms.append((addr, size, name))

text_syms.sort()
print("=== .text from 0x800272F0 to 0x8002AB40 ===")
for addr, size, name in text_syms:
    print(f"0x{addr:08X} (0x{size:04X}) {name}")

# Also check .ctors from 0x802EDD14 to 0x802EDD24
print("\n=== .ctors from 0x802EDD14 to 0x802EDD24 ===")
for line in lines:
    if '.ctors:0x802EDD' in line:
        print(line.strip())
