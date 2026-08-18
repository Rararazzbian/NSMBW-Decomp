import struct
import json
import re
from pathlib import Path

# Load d_basesNP_symbols.txt
syms = []
with open('bin/dtk/d_basesNP_symbols.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        m = re.match(r'^([^=]+)=\s*([^:]+):(0x[0-9a-fA-F]+);\s*//\s*(.*)$', line)
        if m:
            name, sec, addr, comment = m.groups()
            size_m = re.search(r'size:(0x[0-9a-fA-F]+)', comment)
            size = int(size_m.group(1), 16) if size_m else 0
            syms.append((name.strip(), sec.strip(), int(addr, 16), size, comment))

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

# Let's inspect the sections in d_basesNP.rel
with open('slices/d_basesNP.json') as f:
    slice_data = json.load(f)

# Find all known splits in dtk_splits_d_basesNP.txt
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

# Let's map out all functions in .text (sec == '.text')
text_syms = [s for s in syms if s[1] == '.text']
text_syms.sort(key=lambda s: s[2])

print(f"Total .text functions in d_basesNP: {len(text_syms)}")

# Let's inspect the distribution of functions and look for TU boundaries
# A TU boundary in REL is indicated by:
# 1. An entry in dtk_splits_d_basesNP.txt
# 2. A profile definition in .data (g_profile_...)
# 3. An __sinit_ function in .text
# 4. A vtable definition (__vt_...)
# 5. Consecutive functions belonging to an actor class or scene class

# Let's find all profiles in alias_map
profiles_by_data_off = {}
for k, v in alias_map.items():
    if k.startswith('R_2_5_') and 'profile' in v.lower():
        off = int(k.split('_')[3], 16)
        profiles_by_data_off[off] = v

print(f"Total profiles in d_basesNP: {len(profiles_by_data_off)}")

# Let's inspect each profile and find its corresponding code in .text
# In NSMBW actor profiles, a profile struct has pointers:
# struct fActorProfile_c { fProfile::fBaseProfile_c base; u32 m_04; ... }
# base struct has: mpActorClass (class creator fn), executeOrder, drawOrder, ...
