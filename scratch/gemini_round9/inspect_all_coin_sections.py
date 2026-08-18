import sys
import struct
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / 'tools'))
from dolfile import Dol

SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
SPLITS_FILE = ROOT / 'bin' / 'dtk' / 'dtk_splits_wiimj2d.txt'
DOL_FILE = ROOT / 'original' / 'wiimj2d.dol'

lines = SYMS_FILE.read_text().splitlines()
symbols = []
for line in lines:
    m = re.match(r'^(\S+)\s*=\s*(\.\w+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', line.strip())
    if m:
        name, sec, addr_str, meta = m.groups()
        addr = int(addr_str, 16)
        size_m = re.search(r'size:(0x[0-9A-Fa-f]+)', meta)
        size = int(size_m.group(1), 16) if size_m else 0
        symbols.append((sec, addr, size, name))

def list_section_range(sec_name, start_addr, end_addr):
    sec_syms = [s for s in symbols if s[0] == sec_name and start_addr <= s[1] <= end_addr]
    sec_syms.sort(key=lambda s: s[1])
    print(f"\n--- {sec_name} in [0x{start_addr:08X}, 0x{end_addr:08X}] ---")
    for s in sec_syms:
        print(f"  0x{s[1]:08X} - 0x{s[1]+s[2]:08X} (size 0x{s[2]:X}) {s[3]}")

# Let's inspect:
# .rodata around 0x802EE7E0
list_section_range('.rodata', 0x802EE750, 0x802EE830)

# .data around 0x80303078
list_section_range('.data', 0x80302FE0, 0x80303400)

# .sdata2 around 0x8042B630
list_section_range('.sdata2', 0x8042B610, 0x8042B660)

# .bss around 0x803530A8 (carry) - 0x80353400 (door)
list_section_range('.bss', 0x80353090, 0x80353420)

# .sbss around 0x80429F30
list_section_range('.sbss', 0x80429F00, 0x80429F60)

# .sdata around 0x80427B00
list_section_range('.sdata', 0x80427B00, 0x80427B80)
