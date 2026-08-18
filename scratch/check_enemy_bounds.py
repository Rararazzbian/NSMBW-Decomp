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

with open(os.path.join(ROOT, 'bin', 'dtk', 'd_enemiesNP_symbols.txt')) as f:
    sym_lines = f.readlines()

symbols_by_sec = defaultdict(list)
for l in sym_lines:
    m = re.match(r'^(.*?)\s*=\s*(\.[a-zA-Z0-9_$]+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', l.strip())
    if m:
        name, sec, addr_s, rest = m.groups()
        sz_m = re.search(r'size:(0x[0-9A-Fa-f]+|\d+)', rest)
        sz = int(sz_m.group(1), 16 if sz_m.group(1).startswith('0x') else 10) if sz_m else 0
        addr = int(addr_s, 16)
        symbols_by_sec[sec].append((addr, sz, name, rest))

for sec in symbols_by_sec:
    symbols_by_sec[sec].sort()

with open(os.path.join(ROOT, 'scratch', 'd_enemiesNP_tus.json')) as f:
    tus = json.load(f)

# Let's write a function to get full section bounds (.text, .ctors, .rodata, .data, .bss) for any TU
def get_tu_bounds(tu_idx):
    tu = tus[tu_idx]
    t_start = int(tu['start'], 16)
    t_end = int(tu['end'], 16)
    ctor_off = int(tu['ctor_off'], 16)
    
    # ctors is 4 bytes
    ctor_start = ctor_off
    ctor_end = ctor_off + 4
    
    # Find rodata, data, bss symbols referenced by functions in this TU
    tu_relocs = [r for r in all_relocs if r[0] == 1 and t_start <= r[1] < t_end]
    
    ro_addrs = [r[4] for r in tu_relocs if r[2] == 3 and r[3] == 4]
    data_addrs = [r[4] for r in tu_relocs if r[2] == 3 and r[3] == 5]
    bss_addrs = [r[4] for r in tu_relocs if r[2] == 3 and r[3] == 6]
    
    # Also find vtable in data
    # Let's find vtable relocations pointing to functions in this TU
    vt_data_relocs = [r for r in all_relocs if r[0] == 5 and r[2] == 3 and r[3] == 1 and t_start <= r[4] < t_end]
    for r in vt_data_relocs:
        data_addrs.append(r[1])
        
    ro_min = min(ro_addrs) if ro_addrs else None
    ro_max = max(ro_addrs) if ro_addrs else None
    
    data_min = min(data_addrs) if data_addrs else None
    data_max = max(data_addrs) if data_addrs else None
    
    bss_min = min(bss_addrs) if bss_addrs else None
    bss_max = max(bss_addrs) if bss_addrs else None
    
    return {
        'tu_idx': tu_idx,
        'profs': tu['profs'],
        'text': (t_start, t_end),
        'ctors': (ctor_start, ctor_end),
        'rodata': (ro_min, ro_max),
        'data': (data_min, data_max),
        'bss': (bss_min, bss_max)
    }

print('Bound checker ready')
