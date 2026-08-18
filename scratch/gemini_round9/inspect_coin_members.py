import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
disasm_path = ROOT / 'scratch' / 'gemini_round9' / 'coin_main_disasm.txt'
lines = disasm_path.read_text().splitlines()

accesses = []
for line in lines:
    m = re.search(r'(lbz|lhz|lwz|lfs|lfd|stb|sth|stw|stfs|stfd|addic)\s+r\d+,\s*(-?0x[0-9a-fA-F]+|\d+)\((r\d+)\)', line)
    if m:
        inst, offset_str, base_reg = m.groups()
        offset = int(offset_str, 0)
        accesses.append((offset, inst, base_reg, line.strip()))
        
    m2 = re.search(r'addi\s+r\d+,\s*(r\d+),\s*(-?0x[0-9a-fA-F]+|\d+)', line)
    if m2:
        base_reg, offset_str = m2.groups()
        offset = int(offset_str, 0)
        accesses.append((offset, 'addi', base_reg, line.strip()))

high_offsets = sorted(set(a[0] for a in accesses if 0x500 <= a[0] <= 0x1000))
print(f"High offsets accessed (0x500..0x1000): {[hex(x) for x in high_offsets]}")

for off in high_offsets:
    matching = [a for a in accesses if a[0] == off]
    print(f"\nOffset 0x{off:03X} (count={len(matching)}):")
    for a in matching[:5]:
        print(f"  {a[3]}")
