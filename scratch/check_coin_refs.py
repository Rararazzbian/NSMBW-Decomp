import re

with open('scratch/disasm/auto_03_800272F0_text.o.txt') as f:
    for line_no, line in enumerate(f, 1):
        for sym in ['@72501', '@72561', '@72504', '@72351', '@72721']:
            if sym in line:
                print(f"Line {line_no}: {line.strip()}")
