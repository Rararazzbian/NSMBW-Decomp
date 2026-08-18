import os
import sys
import json
import re
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from relfile import Rel, RelSection, RelRelocation

def load_symbols(path):
    syms = []
    sym_re = re.compile(r'^(\S+)\s*=\s*(\.\w+):0x([0-9a-fA-F]+);\s*(?://\s*(.*))?$')
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('//'):
                continue
            m = sym_re.match(line)
            if m:
                name = m.group(1)
                sec = m.group(2)
                addr = int(m.group(3), 16)
                meta = m.group(4) or ''
                size = 0
                sz_m = re.search(r'size:0x([0-9a-fA-F]+)', meta)
                if sz_m:
                    size = int(sz_m.group(1), 16)
                syms.append({'name': name, 'sec': sec, 'addr': addr, 'size': size})
    return syms

def main():
    boss_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'd_en_bossNP_symbols.txt'))
    
    alias_db = {}
    with open(os.path.join(ROOT, 'alias_db.txt'), 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#') and '=' in line:
                k, v = line.split('=', 1)
                alias_db[k.strip()] = v.strip()

    mod4_aliases = {k: v for k, v in alias_db.items() if k.startswith('R_4_')}

    with open(os.path.join(ROOT, 'original', 'd_en_bossNP.rel'), 'rb') as f:
        rel = Rel(4, file=f)

    # Relocations
    relocs_by_src = defaultdict(list)
    for mod_num, relocs in rel.relocations.items():
        curr_pos = 0
        curr_section = 0
        for r in relocs:
            curr_pos += r.offset
            if r.reloc_type.name == 'R_RVL_SECT':
                curr_pos = 0
                curr_section = r.section
                continue
            if r.reloc_type.value < 201:
                relocs_by_src[(curr_section, curr_pos)].append((mod_num, r.section, r.addend, r.reloc_type))

    # All ctors
    ctors_slots = []
    for pos in sorted(relocs_by_src.keys()):
        if pos[0] == 2:
            for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                ctors_slots.append((pos[1], dst_addend))

    # Split starts from DTK
    split_starts = [
        0x110, 0x45dc, 0x666c, 0x9bc4, 0xde30, 0x11738, 0x156d0, 0x1807c,
        0x1c12c, 0x2102c, 0x29118, 0x29efc, 0x2beb4, 0x2dff8, 0x2fbf4,
        0x31e94, 0x34800, 0x377e8, 0x3d038, 0x42798, 0x4635c, 0x4a35c,
        0x4ee48, 0x50744, 0x51bc4, 0x55c94, 0x5702c
    ]

    # Let's extract strings from data / rodata to identify TUs
    data_raw = rel.sections[5].get_data()
    rodata_raw = rel.sections[4].get_data()

    def get_strings(data):
        res = []
        for m in re.finditer(b'([a-zA-Z0-9_./-]{3,})', data):
            s = m.group(1).decode('ascii', errors='ignore')
            res.append((m.start(), s))
        return res

    data_strings = get_strings(data_raw)
    rodata_strings = get_strings(rodata_raw)

    print(f"=== Mapping all 26 TUs in d_en_bossNP.rel ===")
    
    # Let's determine the profiles in .data
    profiles = []
    for k, v in mod4_aliases.items():
        if k.startswith('R_4_5_') and 'profile' in v.lower():
            offset = int(k.split('_')[3], 16)
            profiles.append((offset, v))
    profiles.sort()

    # For each TU i in 0..25:
    # Text range: split_starts[i] to split_starts[i+1]
    # Let's find ctors slot: i * 4
    # Let's find profile associated with this TU
    # Let's find data and rodata and bss ranges
    
    tus = []
    for i in range(26):
        t_start = split_starts[i]
        t_end = split_starts[i+1]
        c_slot = i * 4
        sinit = ctors_slots[i][1] if i < len(ctors_slots) else None
        
        # Find functions in text
        tu_fns = [s for s in boss_syms if s['sec'] == '.text' and t_start <= s['addr'] < t_end]
        code_bytes = sum(f['size'] for f in tu_fns)
        span_bytes = t_end - t_start
        
        # Check symbol coverage: named vs anon
        named_fns = [f for f in tu_fns if not f['name'].startswith('fn_4_') and not f['name'].startswith('gap_') and not f['name'].startswith('pad_')]
        anon_fns = [f for f in tu_fns if f['name'].startswith('fn_4_')]
        
        # Find which profile belongs to this TU
        # Profile factory function points to an address in [t_start, t_end)
        tu_profiles = []
        for p_off, p_name in profiles:
            # Check relocations for this profile
            for p_pos in range(p_off, p_off + 0x14, 4):
                if (5, p_pos) in relocs_by_src:
                    for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[(5, p_pos)]:
                        if dst_mod == 4 and dst_sec == 1 and t_start <= dst_addend < t_end:
                            tu_profiles.append((p_off, p_name))
                            break
        
        # Find strings in rodata/data referenced by this TU
        tu_data_refs = set()
        tu_rodata_refs = set()
        for pos in relocs_by_src:
            if pos[0] == 1 and t_start <= pos[1] < t_end:
                for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                    if dst_mod == 4 and dst_sec == 5:
                        tu_data_refs.add(dst_addend)
                    elif dst_mod == 4 and dst_sec == 4:
                        tu_rodata_refs.add(dst_addend)
                        
        d_min = min(tu_data_refs) if tu_data_refs else 0
        d_max = max(tu_data_refs) if tu_data_refs else 0
        r_min = min(tu_rodata_refs) if tu_rodata_refs else 0
        r_max = max(tu_rodata_refs) if tu_rodata_refs else 0
        
        tus.append({
            'index': i + 1,
            'text_start': t_start,
            'text_end': t_end,
            'code_bytes': code_bytes,
            'span_bytes': span_bytes,
            'fn_count': len(tu_fns),
            'named_fn_count': len(named_fns),
            'anon_fn_count': len(anon_fns),
            'sinit': sinit,
            'profiles': tu_profiles,
            'd_range': (d_min, d_max),
            'r_range': (r_min, r_max),
        })

    for tu in tus:
        p_names = ", ".join([p[1] for p in tu['profiles']]) if tu['profiles'] else "No Profile"
        print(f"TU {tu['index']:2d} | .text: 0x{tu['text_start']:05x}-0x{tu['text_end']:05x} | Span: {tu['span_bytes']:5d} B | Code: {tu['code_bytes']:5d} B | {tu['fn_count']:3d} fns ({tu['named_fn_count']} named, {tu['anon_fn_count']} anon) | Profile: {p_names}")

    with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'boss_all_tus.json'), 'w') as f:
        json.dump(tus, f, indent=2)

if __name__ == '__main__':
    main()
