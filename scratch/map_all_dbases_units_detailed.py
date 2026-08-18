import sys
import os
import struct
import json
import re
from pathlib import Path
from collections import defaultdict

sys.path.append('tools')
from relfile import Rel, PPC_RELOC_TYPE

rel = Rel(2)
with open('original/d_basesNP.rel', 'rb') as f:
    rel.read(f)

# Build complete relocation map
relocs_by_sec = defaultdict(dict)
for mod_id, rlist in rel.relocations.items():
    curr_sec = 0
    curr_off = 0
    for r in rlist:
        curr_off += r.offset
        if r.reloc_type == PPC_RELOC_TYPE.R_RVL_SECT:
            curr_sec = r.section
            curr_off = 0
            continue
        relocs_by_sec[curr_sec][curr_off] = (mod_id, r.section, r.addend, r.reloc_type.name)

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

profiles_list = []
for off, (mod, tsec, addend, rtype) in sorted(relocs_by_sec[5].items()):
    r_key = f"R_2_5_{off:x}"
    if r_key in alias_map:
        pname = alias_map[r_key]
        if pname.startswith('g_profile_'):
            profiles_list.append({
                'profile_name': pname,
                'data_off': off,
                'creator_text_off': addend if (tsec == 1 and mod == 2) else None
            })

print(f"Total profiles identified: {len(profiles_list)}")
for p in profiles_list[:35]:
    c_off_str = f"0x{p['creator_text_off']:06x}" if p['creator_text_off'] is not None else "None"
    print(f"  Profile {p['profile_name']}: .data=0x{p['data_off']:05x}, creator=.text={c_off_str}")

