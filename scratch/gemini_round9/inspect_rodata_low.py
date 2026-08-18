import sys
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'

lines = SYMS_FILE.read_text().splitlines()

rodata_syms = []
for line in lines:
    m = re.match(r'^(\S+)\s*=\s*\.rodata:(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', line.strip())
    if m:
        name, addr_str, meta = m.groups()
        addr = int(addr_str, 16)
        if 0x802EE500 <= addr <= 0x802EE820:
            size_m = re.search(r'size:(0x[0-9A-Fa-f]+)', meta)
            size = int(size_m.group(1), 16) if size_m else 0
            rodata_syms.append((addr, size, name))

rodata_syms.sort()
print("=== .rodata 0x802EE500..0x802EE820 ===")
for addr, size, name in rodata_syms:
    print(f"0x{addr:08X} (0x{size:04X}) {name}")
