import json
import re

# Let us check for any references to d_bg_actor_mng or d_bg_unit in other files
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    text = f.read()

for m in re.finditer(r'__sinit_\\([a-zA-Z0-9_]+)_cpp', text):
    print(m.group(0))
