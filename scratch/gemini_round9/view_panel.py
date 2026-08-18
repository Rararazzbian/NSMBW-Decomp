from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
panel_path = ROOT / 'scratch' / 'gemini_round9' / 'panel_disasm.txt'
lines = panel_path.read_text().splitlines()

print_flag = False
count = 0
for line in lines:
    if '800145B0' in line:
        print_flag = True
    if print_flag:
        print(line)
        count += 1
        if count > 120:
            break
