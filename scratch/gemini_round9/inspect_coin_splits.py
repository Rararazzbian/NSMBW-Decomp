import sys
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SPLITS_FILE = ROOT / 'bin' / 'dtk' / 'dtk_splits_wiimj2d.txt'

lines = SPLITS_FILE.read_text().splitlines()

# Let's search splits for lines around .text 0x800272F0, .data 0x80303078, .rodata 0x802EE7E0, .sdata2 0x8042B630
print("=== SPLITS MATCHES ===")
for i, line in enumerate(lines):
    for addr_str in ['80027', '80028', '80026', '80303', '802EE7', '802EE8', '8042B6']:
        if addr_str in line:
            print(f"Line {i+1:4d}: {line}")
