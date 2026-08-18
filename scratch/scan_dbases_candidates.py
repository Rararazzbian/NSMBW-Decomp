import json
import re
import sys
from pathlib import Path

# Load d_basesNP_symbols.txt
symbols = []
with open('bin/dtk/d_basesNP_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        m = re.match(r'^([^=]+)=\s*([^:]+):(0x[0-9a-fA-F]+);\s*//\s*(.*)$', line)
        if m:
            name, sec, addr, comment = m.groups()
            size_m = re.search(r'size:(0x[0-9a-fA-F]+)', comment)
            size = int(size_m.group(1), 16) if size_m else 0
            type_m = re.search(r'type:(\w+)', comment)
            sym_type = type_m.group(1) if type_m else 'unknown'
            addr_int = int(addr, 16)
            symbols.append({
                'name': name.strip(),
                'sec': sec.strip(),
                'addr': addr_int,
                'size': size,
                'type': sym_type,
                'comment': comment
            })

# Load dtk_splits_d_basesNP.txt
known_splits = {}
curr_file = None
with open('bin/dtk/dtk_splits_d_basesNP.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('Sections:'):
            continue
        if line.endswith(':'):
            curr_file = line[:-1]
            known_splits[curr_file] = {}
        elif '\t' in line or ' ' in line:
            parts = line.split()
            if len(parts) >= 3 and parts[1].startswith('start:') and parts[2].startswith('end:'):
                sec = parts[0]
                s_val = int(parts[1].split(':')[1], 16)
                e_val = int(parts[2].split(':')[1], 16)
                known_splits[curr_file][sec] = (s_val, e_val)

print(f"Known splits in dtk_splits_d_basesNP.txt: {len(known_splits)}")
for k, v in known_splits.items():
    print(f"  {k}: {v.get('.text')}")

# Load alias_db.txt
alias_map = {}
with open('alias_db.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if '=' in line:
            k, v = line.split('=', 1)
            alias_map[k.strip()] = v.strip()

print(f"Loaded {len(alias_map)} aliases from alias_db.txt")
