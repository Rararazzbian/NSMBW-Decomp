import re

with open('scratch/gemini_round3/d_nand_thread_disasm.txt', 'r') as f:
    lines = f.readlines()

# Collect all memory accesses to offsets on registers holding 'this'
offsets = set()
for l in lines:
    m = re.findall(r'(lwz|stw|lbz|stb|lhz|sth)\s+r\d+,\s*(-?0x[0-9a-fA-F]+|\d+)\(r(\d+)\)', l)
    for op, off, reg in m:
        offsets.add((op, off, reg, l.strip()))

# Let's filter for offsets >= 0x50
print("=== Member accesses in d_nand_thread.cpp ===")
for op, off, reg, line in sorted(offsets, key=lambda x: int(x[1], 0) if x[1].startswith('0x') or x[1].isdigit() or (x[1].startswith('-0x')) else 0):
    try:
        val = int(off, 0)
        if 0x50 <= val <= 0x80:
            print(f"{hex(val)} ({op}): {line}")
    except:
        pass
