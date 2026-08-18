import sys
import os
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
SPLITS_FILE = ROOT / 'bin' / 'dtk' / 'dtk_splits_wiimj2d.txt'

lines = SYMS_FILE.read_text().splitlines()

# Search for any symbols containing daEnCoinMain or d_a_en_coin_main
coin_syms = []
for line in lines:
    if 'daEnCoinMain' in line or 'd_a_en_coin_main' in line:
        coin_syms.append(line.strip())

print("=== COIN MAIN SYMBOLS IN SYMBOL FILE ===")
for s in coin_syms:
    print(s)
