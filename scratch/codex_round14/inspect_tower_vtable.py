import os

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
obj_dir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')

with open(os.path.join(obj_dir, 'auto_04_00046BE0_data.txt')) as f:
    lines = f.readlines()
for i, line in enumerate(lines):
    if 'lbl_2_data_480E0' in line:
        for j in range(i, min(len(lines), i+35)):
            print(lines[j], end='')
        break
