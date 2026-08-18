import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
syms_file = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'

for line in syms_file.read_text().splitlines():
    if 'Panel' in line or 'panel' in line:
        print(line)
