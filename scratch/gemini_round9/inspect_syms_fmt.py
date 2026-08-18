import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
syms_file = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'

lines = syms_file.read_text().splitlines()

for line in lines:
    parts = line.strip().split()
    if len(parts) >= 6:
        # e.g. name = 0x8005e900; // type:function size:0x0000b8 scope:global
        # format in wiimj2d_symbols.txt:
        pass

# Let's inspect format of wiimj2d_symbols.txt
for i in range(20):
    print(lines[i])
