import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
syms_file = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
splits_file = ROOT / 'bin' / 'dtk' / 'dtk_splits_wiimj2d.txt'

print("--- SPLITS around d_a_player_manager ---")
for line in splits_file.read_text().splitlines():
    if 'player_manager' in line or '5E900' in line or '61310' in line or '0x8005' in line or '0x8006' in line:
        print(line)

print("\n--- SYMBOLS in wiimj2d_symbols.txt 0x8005E900..0x80061400 ---")
for line in syms_file.read_text().splitlines():
    if '.text:0x8005' in line or '.text:0x8006' in line or '.ctors:0x802E' in line:
        # check address
        for part in line.split():
            if part.startswith('.text:0x8005') or part.startswith('.text:0x8006'):
                addr = int(part.split(':')[1].replace(';', ''), 16)
                if 0x8005E900 <= addr <= 0x80061400:
                    print(line)
