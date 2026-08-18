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

# Let's inspect sections in slices/d_basesNP.json
with open('slices/d_basesNP.json') as f:
    slice_data = json.load(f)

meta_secs = slice_data['meta']['sections']
sec_info = {}
for sname, sinfo in meta_secs.items():
    if sname:
        sec_info[sname] = {
            'index': sinfo['index'],
            'align': sinfo['align'],
            'size': int(sinfo['size'], 16)
        }

# Build map of relocations
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

# Let's write a helper to find exact section bounds for any candidate TU
# In a REL TU:
# .text: [t_start, t_end)
# .ctors: [c_start, c_end)
# .rodata: [r_start, r_end)
# .data: [d_start, d_end)
# .bss: [b_start, b_end)

def audit_tu_bounds(t_start, t_end, d_start, d_end, r_start, r_end, c_start, c_end, b_start, b_end):
    # Check bounds against all other known slices
    pass

