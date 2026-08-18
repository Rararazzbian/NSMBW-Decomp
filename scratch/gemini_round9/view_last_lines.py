import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
disasm_path = ROOT / 'scratch' / 'gemini_round9' / 'coin_main_disasm.txt'
lines = disasm_path.read_text().splitlines()

for i in range(len(lines)-30, len(lines)):
    print(lines[i])
