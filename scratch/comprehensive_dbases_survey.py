import sys
import os
import re
import json
from pathlib import Path
from collections import defaultdict

sys.path.append('tools')
from relfile import Rel, PPC_RELOC_TYPE

rel = Rel(2)
with open('original/d_basesNP.rel', 'rb') as f:
    rel.read(f)

print("Parsed d_basesNP.rel successfully!")

# Parse relocations to find all symbols pointed to
# Map of (sec, off) -> (target_mod, target_sec, target_addend)
reloc_map = {}
for mod_id, rlist in rel.relocations.items():
    curr_sec = 0
    curr_off = 0
    for r in rlist:
        curr_off += r.offset
        if r.reloc_type == PPC_RELOC_TYPE.R_RVL_SECT:
            curr_sec = r.section
            curr_off = 0
            continue
        reloc_map[(curr_sec, curr_off)] = (mod_id, r.section, r.addend, r.reloc_type.name)

# 1. CTORS relocations (section 2)
print("\n--- CTORS RELOCATIONS ---")
ctors_sinits = []
for (sec, off), (mod, tsec, addend, rtype) in sorted(reloc_map.items()):
    if sec == 2: # .ctors
        # addend is the offset in section 1 (.text) of __sinit_
        ctors_sinits.append((off, addend))
        print(f"  .ctors +0x{off:03x} -> .text +0x{addend:06x}")

print(f"Total .ctors entries: {len(ctors_sinits)}")

# Load d_basesNP_symbols.txt
symbols = []
with open('bin/dtk/d_basesNP_symbols.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        m = re.match(r'^([^=]+)=\s*([^:]+):(0x[0-9a-fA-F]+);\s*//\s*(.*)$', line)
        if m:
            name, sname, addr, comment = m.groups()
            size_m = re.search(r'size:(0x[0-9a-fA-F]+)', comment)
            size = int(size_m.group(1), 16) if size_m else 0
            symbols.append({
                'name': name.strip(),
                'sec': sname.strip(),
                'addr': int(addr, 16),
                'size': size,
                'comment': comment
            })

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

# Load slices/d_basesNP.json
with open('slices/d_basesNP.json') as f:
    d_bases_json = json.load(f)

landed_slices = [s for s in d_bases_json['slices'] if s.get('source') and not s.get('nonMatching')]
print(f"\nLanded slices in d_basesNP: {len(landed_slices)}")
for s in landed_slices:
    print(f"  {s['source']}: {s.get('memoryRanges')}")

