import re

with open('bin/dtk/wiimj2d_symbols.txt', 'r') as f:
    lines = f.readlines()

for line in lines:
    if 'tmpSave' in line or '800CFCA' in line or '361F' in line or '361' in line or '3710' in line:
        print(line.strip())
