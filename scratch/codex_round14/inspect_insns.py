import os
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import sibmap

dis_dir = os.path.join(ROOT, 'scratch', 'codex_round14')

print("=== Functions in d_a_wm_grid.cpp (0x164230 - 0x164430) ===")
grid_files = ['auto_00_00164204_text.txt', 'auto_fn_2_164380_text.txt', 'auto_00_00164404_text.txt']
for f in grid_files:
    p = os.path.join(dis_dir, f)
    fns = sibmap.parse(p)
    for fn in fns:
        if 0x164230 <= fn['addr'] < 0x164430:
            print(f"0x{fn['addr']:06x} ({len(fn['words']):3d} insns / {len(fn['words'])*4:4d} B) {fn['name']}")
            for insn in fn['texts']:
                print(f"    {insn}")

print("\n=== Functions in d_a_wm_tower.cpp (0x185710 - 0x185b70) ===")
tower_files = ['auto_00_001856E4_text.txt', 'auto_fn_2_185AC0_text.txt', 'auto_00_00185B44_text.txt']
for f in tower_files:
    p = os.path.join(dis_dir, f)
    fns = sibmap.parse(p)
    for fn in fns:
        if 0x185710 <= fn['addr'] < 0x185b70:
            print(f"0x{fn['addr']:06x} ({len(fn['words']):3d} insns / {len(fn['words'])*4:4d} B) {fn['name']}")
            for insn in fn['texts']:
                print(f"    {insn}")
