import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
disasm_path = ROOT / 'scratch' / 'gemini_round9' / 'coin_main_disasm.txt'
disasm_lines = disasm_path.read_text().splitlines()

# Print lines from 0x80028070 to the end
print_flag = False
for line in disasm_lines:
    if '80028070' in line:
        print_flag = True
    if print_flag:
        print(line)
