import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
target_text_file = ROOT / 'wip' / 'player_manager' / 'target_text.txt'

lines = target_text_file.read_text().splitlines()
fns = []
for line in lines:
    if line.startswith('.fn '):
        parts = line.split(',')
        fn_name = parts[0].replace('.fn', '').strip().strip('"')
        fns.append(fn_name)

print(f"Total functions in target_text.txt: {len(fns)}")
for i, fn in enumerate(fns):
    print(f"  {i+1}: {fn}")
