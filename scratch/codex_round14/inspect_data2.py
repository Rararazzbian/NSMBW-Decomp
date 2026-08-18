import os

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
obj_dir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')

print("=== Grid Data (around 0x44cb4) ===")
with open(os.path.join(obj_dir, 'auto_04_00044A68_data.txt')) as f:
    lines = f.readlines()
for i, line in enumerate(lines):
    if '44cb4' in line.lower() or '44cc0' in line.lower() or '44c90' in line.lower():
        for j in range(max(0, i-2), min(len(lines), i+45)):
            print(lines[j], end='')
        break

print("\n=== Tower Data (around 0x480b4) ===")
with open(os.path.join(obj_dir, 'auto_04_00046BE0_data.txt')) as f:
    lines = f.readlines()
for i, line in enumerate(lines):
    if '480b4' in line.lower() or '480e0' in line.lower() or '48090' in line.lower():
        for j in range(max(0, i-2), min(len(lines), i+45)):
            print(lines[j], end='')
        break
