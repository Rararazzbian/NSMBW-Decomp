import os
import subprocess

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
dtk = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
dis_dir = os.path.join(ROOT, 'scratch', 'codex_round14')
os.makedirs(dis_dir, exist_ok=True)

# 1. d_a_wm_grid.cpp split objects
grid_objs = ['auto_00_00164204_text.o', 'auto_fn_2_164380_text.o', 'auto_00_00164404_text.o']
for obj in grid_objs:
    src = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', obj)
    out = os.path.join(dis_dir, obj.replace('.o', '.txt'))
    subprocess.run([dtk, 'elf', 'disasm', src, out], check=True)

# 2. d_a_wm_tower.cpp split objects
tower_objs = ['auto_00_001856E4_text.o', 'auto_fn_2_185AC0_text.o', 'auto_00_00185B44_text.o']
for obj in tower_objs:
    src = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', obj)
    out = os.path.join(dis_dir, obj.replace('.o', '.txt'))
    subprocess.run([dtk, 'elf', 'disasm', src, out], check=True)

print("Disassembly complete for grid and tower!")
