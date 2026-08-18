import os
import re
import sys

sys.path.insert(0, os.path.abspath('.'))

# Parse dtk_splits_wiimj2d.txt
with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r', encoding='utf-8') as f:
    lines = f.readlines()

units = []
current_unit = None

for line in lines:
    line = line.rstrip()
    if not line or line.startswith('Sections:'):
        continue
    if not line.startswith('\t') and line.endswith(':'):
        unit_name = line[:-1]
        current_unit = {'name': unit_name, 'sections': {}}
        units.append(current_unit)
    elif line.startswith('\t') and current_unit is not None:
        parts = line.strip().split()
        sec_name = parts[0]
        sec_info = {}
        for p in parts[1:]:
            if ':' in p:
                k, v = p.split(':', 1)
                sec_info[k] = int(v, 16) if v.startswith('0x') else v
        current_unit['sections'][sec_name] = sec_info

print(f"Total units defined in dtk_splits: {len(units)}")

# Check which ones exist in source/
missing_units = []
for u in units:
    source_path = os.path.join('source', u['name'])
    if not os.path.exists(source_path):
        missing_units.append(u)

print(f"Units NOT yet in source/: {len(missing_units)}")
for u in missing_units[:20]:
    print(u['name'], u['sections'].get('.text'))
