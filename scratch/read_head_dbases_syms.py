import re

with open('bin/dtk/d_basesNP_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    lines = [f.readline() for _ in range(50)]
    print(''.join(lines))
