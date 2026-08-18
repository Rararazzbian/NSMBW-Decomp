import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
disasm_path = ROOT / 'scratch' / 'gemini_round9' / 'coin_main_disasm.txt'
disasm_text = disasm_path.read_text()

# Extract all functions
fns = []
curr_fn = None
curr_lines = []

for line in disasm_text.splitlines():
    m = re.match(r'^\.fn\s+"?([^",]+)"?,\s*(\w+)', line.strip())
    if m:
        curr_fn = m.group(1)
        curr_lines = []
    elif line.strip() == '.endfn':
        if curr_fn:
            fns.append((curr_fn, curr_lines))
            curr_fn = None
    elif curr_fn:
        curr_lines.append(line)

print(f"Total functions extracted from disassembly: {len(fns)}")
for name, lines in fns:
    print(f"  - {name} ({len(lines)} lines)")
