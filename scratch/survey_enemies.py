import struct
import re
import os
import json

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

with open(os.path.join(ROOT, 'original', 'd_enemiesNP.rel'), 'rb') as f:
    rel_data = f.read()

num_secs, sec_info_off = struct.unpack('>II', rel_data[0xC:0x14])
sections = []
for i in range(num_secs):
    off, sz = struct.unpack('>II', rel_data[sec_info_off + i*8 : sec_info_off + i*8 + 8])
    is_exec = (off & 1) != 0
    off = off & ~1
    sections.append((i, off, sz, is_exec))

# Sections:
# 1: .text (sz=0x12a428)
# 2: .ctors (sz=0x288)
# 3: .dtors (sz=0x4)
# 4: .rodata (sz=0x8610)
# 5: .data (sz=0x3b678)
# 6: .bss (sz=0xb4e0)

imp_offset, imp_size = struct.unpack('>II', rel_data[0x28:0x30])
imports = []
for i in range(imp_size // 8):
    mod_id, rel_off = struct.unpack('>II', rel_data[imp_offset + i*8 : imp_offset + i*8 + 8])
    imports.append((mod_id, rel_off))

def parse_relocs(mod_id, start_off):
    off = start_off
    cur_sec = 0
    cur_sec_off = 0
    relocs = []
    while True:
        prev_off, rtype, rsec, raddend = struct.unpack('>HBB I', rel_data[off:off+8])
        off += 8
        cur_sec_off += prev_off
        if rtype == 203:
            break
        elif rtype == 202:
            cur_sec = rsec
            cur_sec_off = 0
        else:
            relocs.append((cur_sec, cur_sec_off, mod_id, rsec, raddend, rtype))
    return relocs

all_relocs = []
for mod_id, r_off in imports:
    all_relocs.extend(parse_relocs(mod_id, r_off))

ctor_relocs = [r for r in all_relocs if r[0] == 2]
ctor_relocs.sort(key=lambda r: r[1])

# Load symbols from d_enemiesNP_symbols.txt
with open(os.path.join(ROOT, 'bin', 'dtk', 'd_enemiesNP_symbols.txt')) as f:
    sym_lines = f.readlines()

text_syms = []
data_syms = {}
rodata_syms = {}
bss_syms = {}

for l in sym_lines:
    m = re.match(r'^(.*?)\s*=\s*(\.[a-zA-Z0-9_$]+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', l.strip())
    if m:
        name, sec, addr_s, rest = m.groups()
        sz_m = re.search(r'size:(0x[0-9A-Fa-f]+|\d+)', rest)
        sz = int(sz_m.group(1), 16 if sz_m.group(1).startswith('0x') else 10) if sz_m else 0
        addr = int(addr_s, 16)
        if sec == '.text':
            text_syms.append((addr, sz, name))
        elif sec == '.data':
            data_syms[addr] = (name, sz, rest)
        elif sec == '.rodata':
            rodata_syms[addr] = (name, sz, rest)
        elif sec == '.bss':
            bss_syms[addr] = (name, sz, rest)

text_syms.sort()

# Profiles in .data
prof_by_class_init = {}
for r in all_relocs:
    if r[0] == 5 and r[2] == 3 and r[3] == 1:
        if r[1] in data_syms and data_syms[r[1]][0].startswith('g_profile_'):
            prof_by_class_init[r[4]] = (r[1], data_syms[r[1]][0])

# Map out each TU
tus = []
cur_idx = 0
for c_idx, c_rel in enumerate(ctor_relocs):
    sinit_addr = c_rel[4]
    sinit_idx = None
    for i in range(cur_idx, len(text_syms)):
        if text_syms[i][0] == sinit_addr:
            sinit_idx = i
            break
    if sinit_idx is None:
        continue
    
    # Check for static dtors registered in sinit
    end_idx = sinit_idx
    # Let's see if there are dtors after sinit belonging to this TU:
    # Check relocations in sinit
    sinit_sz = text_syms[sinit_idx][1]
    sinit_relocs = [r for r in all_relocs if r[0] == 1 and sinit_addr <= r[1] < sinit_addr + sinit_sz]
    
    # Any functions called or referenced as dtors in sinit?
    dtor_addrs = set()
    for r in sinit_relocs:
        if r[2] == 3 and r[3] == 1: # mod 3, sec 1
            if r[4] > sinit_addr:
                dtor_addrs.add(r[4])
    
    while end_idx + 1 < len(text_syms) and text_syms[end_idx + 1][0] in dtor_addrs:
        end_idx += 1
    
    tu_fns = text_syms[cur_idx : end_idx + 1]
    tu_start = tu_fns[0][0]
    last_fn = text_syms[end_idx]
    tu_end = (last_fn[0] + last_fn[1] + 15) & ~15
    tu_span = tu_end - tu_start
    tu_code = sum(fn[1] for fn in tu_fns)
    
    # Find profile names
    profs = []
    for fn in tu_fns:
        if fn[0] in prof_by_class_init:
            profs.append(prof_by_class_init[fn[0]])
            
    tus.append({
        'tu_idx': c_idx,
        'ctor_off': c_rel[1],
        'start': tu_start,
        'end': tu_end,
        'span': tu_span,
        'code': tu_code,
        'fns': len(tu_fns),
        'sinit': sinit_addr,
        'profs': profs,
        'first_fn': tu_fns[0][2],
        'fn_list': tu_fns
    })
    cur_idx = end_idx + 1

print(f'Total TUs found: {len(tus)}')

# Save TU list to JSON
with open(os.path.join(ROOT, 'scratch', 'd_enemiesNP_tus.json'), 'w') as f:
    json.dump([{
        'tu_idx': t['tu_idx'],
        'ctor_off': hex(t['ctor_off']),
        'start': hex(t['start']),
        'end': hex(t['end']),
        'span': t['span'],
        'code': t['code'],
        'fns': t['fns'],
        'profs': [p[1] for p in t['profs']]
    } for t in tus], f, indent=2)

for tu in tus[:30]:
    p_names = ', '.join([p[1] for p in tu['profs']]) if tu['profs'] else 'NO_PROFILE'
    print(f"TU {tu['tu_idx']:2d} (.ctors+{hex(tu['ctor_off'])}): .text {hex(tu['start'])}-{hex(tu['end'])} (span {tu['span']} B, code {tu['code']} B, {tu['fns']:2d} fns) -> {p_names}")
