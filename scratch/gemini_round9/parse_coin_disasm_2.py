import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
disasm_path = ROOT / 'scratch' / 'gemini_round9' / 'coin_main_disasm.txt'
disasm_text = disasm_path.read_text()

# Extract all functions
fns = []
curr_fn = None
curr_comment = None
curr_lines = []

for line in disasm_text.splitlines():
    line_s = line.strip()
    if line_s.startswith('# daEnCoinMain_c::') or line_s.startswith('# '):
        curr_comment = line_s
    m = re.match(r'^\.fn\s+([^,]+),\s*(\w+)', line_s)
    if m:
        curr_fn = m.group(1).strip()
        curr_lines = []
    elif line_s.startswith('.endfn'):
        if curr_fn:
            fns.append((curr_fn, curr_comment, curr_lines))
            curr_fn = None
            curr_comment = None
    elif curr_fn:
        curr_lines.append(line_s)

print(f"Total functions extracted: {len(fns)}")
for name, comment, lines in fns:
    if not name.startswith('gap_'):
        print(f"\nFunction: {name}")
        if comment: print(f"  Comment: {comment}")
        print(f"  Instructions count: {len(lines)}")
