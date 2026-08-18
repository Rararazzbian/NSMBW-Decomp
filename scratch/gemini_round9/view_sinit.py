from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
disasm_path = ROOT / 'scratch' / 'gemini_round9' / 'coin_main_disasm.txt'
lines = disasm_path.read_text().splitlines()

print_flag = False
for line in lines:
    if '80028150' in line:
        print_flag = True
    if print_flag:
        print(line)
