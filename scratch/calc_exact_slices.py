import struct
import re
import os
import json
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

def get_exact_tu_slices(rel_name, tu_candidates):
    sym_file = rel_name.replace('.rel', '_symbols.txt')
    with open(os.path.join(ROOT, 'bin', 'dtk', sym_file)) as f:
        sym_lines = f.readlines()

    syms_by_sec = defaultdict(list)
    for l in sym_lines:
        m = re.match(r'^(.*?)\s*=\s*(\.[a-zA-Z0-9_$]+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', l.strip())
        if m:
            name, sec, addr_s, rest = m.groups()
            sz_m = re.search(r'size:(0x[0-9A-Fa-f]+|\d+)', rest)
            sz = int(sz_m.group(1), 16 if sz_m.group(1).startswith('0x') else 10) if sz_m else 0
            addr = int(addr_s, 16)
            syms_by_sec[sec].append((addr, sz, name, rest))

    for sec in syms_by_sec:
        syms_by_sec[sec].sort()

    # Load REL relocs
    rel_path = os.path.join(ROOT, 'original', rel_name)
    with open(rel_path, 'rb') as f:
        rel_data = f.read()

    imp_offset, imp_size = struct.unpack('>II', rel_data[0x28:0x30])

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
    for i in range(imp_size // 8):
        m_id, r_off = struct.unpack('>II', rel_data[imp_offset + i*8 : imp_offset + i*8 + 8])
        all_relocs.extend(parse_relocs(m_id, r_off))

    for tu in tu_candidates:
        t_start, t_end = tu['text']
        c_start, c_end = tu['ctors']
        
        # Find all vtables that point into this text range
        vt_data_addrs = [r[1] for r in all_relocs if r[0] == 5 and t_start <= r[4] < t_end]
        
        # Profiles in this TU
        prof_data_addrs = []
        for addr, sz, name, rest in syms_by_sec['.data']:
            if any(p in name for p in tu.get('profs', [])):
                prof_data_addrs.append(addr)
                
        # In .data, TU starts at lowest profile/symbol and ends at end of vtable / static data
        data_all = vt_data_addrs + prof_data_addrs
        if data_all:
            data_min = min(data_all)
            # Find the symbol at data_min and symbols up to max(data_all)
            # In RELs, data symbols within TU are contiguous
            # Find end of highest symbol in this group
            data_max = max(data_all)
            sz = 4
            for a, s, n, r in syms_by_sec['.data']:
                if a == data_max:
                    sz = s
            data_end = (data_max + sz + 7) & ~7
            data_start = data_min & ~7
        else:
            data_start, data_end = 0, 0
            
        # For .rodata: find rodata referenced ONLY between data_start..data_end and t_start..t_end
        tu_text_relocs = [r for r in all_relocs if r[0] == 1 and t_start <= r[1] < t_end]
        tu_data_relocs = [r for r in all_relocs if r[0] == 5 and data_start <= r[1] < data_end]
        
        ro_candidates = [r[4] for r in tu_text_relocs + tu_data_relocs if r[2] == tu['mod_id'] and r[3] == 4]
        if ro_candidates:
            # check which rodata symbols are contiguous
            ro_min = min(ro_candidates)
            ro_max = max(ro_candidates)
            sz = 4
            for a, s, n, r in syms_by_sec['.rodata']:
                if a == ro_max:
                    sz = s
            ro_end = (ro_max + sz + 7) & ~7
            ro_start = ro_min & ~7
        else:
            ro_start, ro_end = 0, 0
            
        # For .bss: find bss referenced in text / data / __sinit
        bss_candidates = [r[4] for r in tu_text_relocs + tu_data_relocs if r[2] == tu['mod_id'] and r[3] == 6]
        if bss_candidates:
            bss_min = min(bss_candidates)
            bss_max = max(bss_candidates)
            sz = 4
            for a, s, n, r in syms_by_sec['.bss']:
                if a == bss_max:
                    sz = s
            bss_end = (bss_max + sz + 7) & ~7
            bss_start = bss_min & ~7
        else:
            bss_start, bss_end = 0, 0
            
        print(f"=== {tu['name']} ===")
        print(f'  .text   : 0x{t_start:x}-0x{t_end:x} (Span: {t_end-t_start} B)')
        print(f'  .ctors  : 0x{c_start:x}-0x{c_end:x}')
        print(f'  .rodata : 0x{ro_start:x}-0x{ro_end:x} (Span: {ro_end-ro_start} B)')
        print(f'  .data   : 0x{data_start:x}-0x{data_end:x} (Span: {data_end-data_start} B)')
        print(f'  .bss    : 0x{bss_start:x}-0x{bss_end:x} (Span: {bss_end-bss_start} B)')

print('--- d_basesNP.rel ---')
get_exact_tu_slices('d_basesNP.rel', [{
    'name': 'd_a_wm_kinoko_base.cpp',
    'mod_id': 2,
    'text': (0x16b2d0, 0x16bda0),
    'ctors': (0x3fc, 0x400),
    'profs': ['g_profile_WM_KINOKO_BASE']
}])

print('\n--- d_enemiesNP.rel Top 8 Candidates ---')
enemies_cands = [
    {'name': 'd_a_en_net_nokonoko_lr.cpp', 'mod_id': 3, 'text': (0xcd5b0, 0xce330), 'ctors': (0x1ac, 0x1b0), 'profs': ['g_profile_EN_NET_NOKONOKO_LR']},
    {'name': 'd_a_en_left_dokan_pakkun.cpp', 'mod_id': 3, 'text': (0x502e0, 0x50de0), 'ctors': (0xc0, 0xc4), 'profs': ['g_profile_EN_LEFT_DOKAN_PAKKUN']},
    {'name': 'd_a_en_icebros.cpp', 'mod_id': 3, 'text': (0x7cf40, 0x7dae0), 'ctors': (0x124, 0x128), 'profs': ['g_profile_EN_ICEBROS']},
    {'name': 'd_a_en_jimen_pakkun.cpp', 'mod_id': 3, 'text': (0x90930, 0x91190), 'ctors': (0x144, 0x148), 'profs': ['g_profile_EN_JIMEN_PAKKUN']},
    {'name': 'd_a_en_block_cloud.cpp', 'mod_id': 3, 'text': (0x1f300, 0x1ff20), 'ctors': (0x44, 0x48), 'profs': ['g_profile_EN_BLOCK_CLOUD']},
    {'name': 'd_a_en_super_bigpile_left.cpp', 'mod_id': 3, 'text': (0x108520, 0x109100), 'ctors': (0x230, 0x234), 'profs': ['g_profile_EN_SUPER_BIGPILE_LEFT']},
    {'name': 'd_a_en_waki_jugem.cpp', 'mod_id': 3, 'text': (0x119990, 0x11a130), 'ctors': (0x25c, 0x260), 'profs': ['g_profile_EN_WAKI_JUGEM']},
    {'name': 'd_a_en_coin_jump.cpp', 'mod_id': 3, 'text': (0x44620, 0x44ef0), 'ctors': (0x94, 0x98), 'profs': ['g_profile_EN_COIN_JUMP']},
]
get_exact_tu_slices('d_enemiesNP.rel', enemies_cands)
