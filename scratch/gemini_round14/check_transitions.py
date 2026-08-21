import re

with open('scratch/gemini_round14/task_a_all_text.txt', 'r', encoding='utf-8') as f:
    lines = [l.strip() for l in f]

# Let's see what symbols are around each sinit
for i, line in enumerate(lines):
    if '__sinit' in line:
        start = max(0, i - 5)
        end = min(len(lines), i + 8)
        print("="*60)
        print(f"Around {line}:")
        for j in range(start, end):
            prefix = ">>> " if j == i else "    "
            print(f"{prefix}{lines[j]}")
