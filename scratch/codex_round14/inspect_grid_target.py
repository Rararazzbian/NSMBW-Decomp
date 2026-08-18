import os
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import sibmap

scratch_dir = os.path.join(ROOT, 'scratch', 'codex_round14')
p = os.path.join(scratch_dir, 'auto_00_00164204_text.txt')
fns = sibmap.parse(p)
for fn in fns:
    if fn['addr'] in [0x164230, 0x164260, 0x1642b0, 0x164370]:
        print(f"=== Function 0x{fn['addr']:06x} ({fn['name']}) ===")
        for insn in fn['texts']:
            print("   ", insn)
