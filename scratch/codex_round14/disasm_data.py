import os
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
dtk = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
dis_dir = os.path.join(ROOT, 'scratch', 'codex_round14')

# Disassemble the data object auto_04_00044A68_data.o
src = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_04_00044A68_data.o')
out = os.path.join(dis_dir, 'auto_04_00044A68_data.txt')
if not os.path.exists(out):
    os.system(f'"{dtk}" elf disasm "{src}" "{out}" >nul 2>&1')

# Also data object for tower: auto_04_00046BE0_data.o
src_tower = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_04_00046BE0_data.o')
out_tower = os.path.join(dis_dir, 'auto_04_00046BE0_data.txt')
if not os.path.exists(out_tower):
    os.system(f'"{dtk}" elf disasm "{src_tower}" "{out_tower}" >nul 2>&1')

print("Disassembled data objects!")
