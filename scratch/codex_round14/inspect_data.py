import os

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
dis_dir = os.path.join(ROOT, 'scratch', 'codex_round14')

print("=== Grid Data (around 0x44cb4) ===")
with open(os.path.join(dis_dir, 'auto_04_00044A68_data.txt')) as f:
    lines = f.readlines()
for i, line in enumerate(lines):
    if '00044CB4' in line or '00044cb4' in line or '00044CC0' in line or '00044cc0' in line:
        for j in range(max(0, i-2), min(len(lines), i+45)):
            print(lines[j], end='')
        break

print("\n=== Tower Data (around 0x480b4) ===")
with open(os.path.join(dis_dir, 'auto_04_00046BE0_data.txt')) as f:
    lines = f.readlines()
for i, line in enumerate(lines):
    if '000480B4' in line or '000480b4' in line or '000480E0' in line or '000480e0' in line:
        for j in range(max(0, i-2), min(len(lines), i+45)):
            print(lines[j], end='')
        break
