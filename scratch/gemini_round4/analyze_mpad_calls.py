import re
import os

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"

text_files = [
    os.path.join(ROOT, "scratch", "gemini_round4", "orig_mpad_text.txt"),
    os.path.join(ROOT, "scratch", "gemini_round4", "orig_mpad_text2.txt")
]

calls = set()
for tf in text_files:
    with open(tf) as f:
        for line in f:
            line = line.strip()
            m = re.search(r'\bbl\s+([^\s\n]+)', line)
            if m:
                calls.add(m.group(1))

# Check each symbol against wiimj2d_symbols.txt and include headers
sym_info = {}
with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')) as f:
    for line in f:
        for c in calls:
            if line.startswith(c + " ="):
                sym_info[c] = line.strip()

print(f"Total external/internal calls found: {len(calls)}")
for c in sorted(calls):
    info = sym_info.get(c, "MISSING_FROM_SYMS")
    print(f"{c:50} | {info}")
