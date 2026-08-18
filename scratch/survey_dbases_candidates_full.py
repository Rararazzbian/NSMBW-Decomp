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

# Load symbols from d_basesNP_symbols.txt
symbols_by_sec = defaultdict(list)
all_syms = []
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
            obj = {
                'name': name.strip(),
                'sec': sname.strip(),
                'addr': int(addr, 16),
                'size': size,
                'comment': comment
            }
            symbols_by_sec[sname.strip()].append(obj)
            all_syms.append(obj)

for sec in symbols_by_sec:
    symbols_by_sec[sec].sort(key=lambda s: s['addr'])

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

# Build relocation map
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

# Collect all .ctors entries and their targets
ctors_list = []
for off, (mod, tsec, addend, rtype) in sorted(relocs_by_sec[2].items()):
    ctors_list.append((off, addend))

# Collect all profiles
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

# Let's inspect known landed slices in slices/d_basesNP.json
with open('slices/d_basesNP.json') as f:
    d_bases_json = json.load(f)

landed_slices = [s for s in d_bases_json['slices'] if s.get('source') and not s.get('nonMatching')]

print(f"Total profiles: {len(profiles_list)}, total ctors: {len(ctors_list)}, landed slices: {len(landed_slices)}")

# Let's inspect some clean actor TUs in d_basesNP:
# Look for actors that are compact, clean, well-bounded, derived from standard classes (dEn_c, dActor_c, dStageActor_c, dWmDemoActor_c, etc.)
# Let's search across all profiles and inspect their function spans in .text!

for i in range(len(profiles_list)):
    p = profiles_list[i]
    c_off = p['creator_text_off']
    if c_off is None:
        continue
    # Find next profile's creator
    next_c_off = None
    for j in range(i + 1, len(profiles_list)):
        if profiles_list[j]['creator_text_off'] is not None:
            next_c_off = profiles_list[j]['creator_text_off']
            break
    
    # Calculate span in text
    if next_c_off:
        span_bytes = next_c_off - c_off
    else:
        span_bytes = len(rel.sections[1]._data) - c_off
        
    # Count functions in this text range
    fns = [s for s in symbols_by_sec['.text'] if c_off <= s['addr'] < (next_c_off if next_c_off else 0x1C6004)]
    code_bytes = sum(s['size'] for s in fns)
    
    # Check if there is an __sinit_ in this range
    sinit_in_range = [c for c in ctors_list if c_off <= c[1] < (next_c_off if next_c_off else 0x1C6004)]
    
    if 200 <= span_bytes <= 10000 and len(fns) <= 40:
        print(f"Candidate: {p['profile_name']}")
        print(f"  .text: 0x{c_off:06x}..0x{(next_c_off if next_c_off else 0x1C6004):06x} (span {span_bytes} B, code {code_bytes} B, {len(fns)} fns)")
        print(f"  .data profile off: 0x{p['data_off']:05x}, sinit ctors: {len(sinit_in_range)}")
