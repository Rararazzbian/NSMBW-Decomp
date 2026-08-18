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
            if not line or line.startswith('#') or line.startswith('//'): continue
            m = sym_re.match(line)
            if m:
                name = m.group(1)
                sec = m.group(2)
                addr = int(m.group(3), 16)
                meta = m.group(4) or ''
                size = 0
                sz_m = re.search(r'size:0x([0-9a-fA-F]+)', meta)
                if sz_m: size = int(sz_m.group(1), 16)
                stype = 'unknown'
                st_m = re.search(r'type:(\w+)', meta)
                if st_m: stype = st_m.group(1)
                scope = 'local'
                sc_m = re.search(r'scope:(\w+)', meta)
                if sc_m: scope = sc_m.group(1)
                syms.append({
                    'name': name, 'sec': sec, 'addr': addr, 'size': size,
                    'type': stype, 'scope': scope,
                })
    return syms

def sec_to_idx(sec):
    mapping = {'': 0, '.text': 1, '.ctors': 2, '.dtors': 3, '.rodata': 4, '.data': 5, '.bss': 6}
    return mapping.get(sec, 0)

def idx_to_sec(idx):
    mapping = {0: '', 1: '.text', 2: '.ctors', 3: '.dtors', 4: '.rodata', 5: '.data', 6: '.bss'}
    return mapping.get(idx, f'sec_{idx}')

def main():
    alias_db = {}
    with open(os.path.join(ROOT, 'alias_db.txt'), 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'): continue
            if '=' in line:
                k, v = line.split('=', 1)
                alias_db[k.strip()] = v.strip()

    syms_txt = {}
    with open(os.path.join(ROOT, 'syms.txt'), 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'): continue
            if '=' in line:
                k, v = line.split('=', 1)
                syms_txt[k.strip()] = int(v.strip(), 16)

    dol_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'))
    dol_syms_by_addr = {(s['sec'], s['addr']): s for s in dol_syms}
    dol_syms_by_name = {s['name']: s for s in dol_syms}

    bases_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt'))
    bases_syms_by_addr = {(s['sec'], s['addr']): s for s in bases_syms}
    bases_syms_by_name = {s['name']: s for s in bases_syms}

    with open(os.path.join(ROOT, 'slices', 'wiimj2d.json')) as f:
        dol_slices_data = json.load(f)
    dol_landed_slices = [s for s in dol_slices_data['slices'] if s.get('source') and not s.get('nonMatching')]
    
    with open(os.path.join(ROOT, 'slices', 'd_basesNP.json')) as f:
        bases_slices_data = json.load(f)
    bases_landed_slices = [s for s in bases_slices_data['slices'] if s.get('source') and not s.get('nonMatching')]

    rel_path = os.path.join(ROOT, 'original', 'd_basesNP.rel')
    with open(rel_path, 'rb') as f:
        rel = Rel(2, file=f)

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

    exact_targets = {
        'd_basesNP/bases/d_a_wm_grid.cpp': {
            1: (0x164210, 0x164404),
            2: (0x3e4, 0x3e8),
            4: (0x88b8, 0x88c8),
            5: (0x44c90, 0x44d20),
            6: (0xfdd0, 0xfde0),
        },
        'd_basesNP/bases/d_a_wm_tower.cpp': {
            1: (0x1856f0, 0x185b44),
            2: (0x44c, 0x450),
            4: (0x9320, 0x9330),
            5: (0x48090, 0x48158),
            6: (0x10350, 0x10360),
        }
    }

    landing_kits = {}

    for tu_name, sec_ranges in exact_targets.items():
        print(f"\n==============================================")
        print(f"=== Landing Kit Analysis: {tu_name} ===")
        print(f"==============================================")

        # 1. Target symbols within TU (these are to be replaced / removed from d_basesNP_symbols.txt if landing removes them, or kept)
        owned_symbols = []
        for sec_idx, (start, end) in sec_ranges.items():
            sec_name = idx_to_sec(sec_idx)
            for s in bases_syms:
                if s['sec'] == sec_name and start <= s['addr'] < end:
                    owned_symbols.append(s)

        print(f"Owned symbols ({len(owned_symbols)}):")
        for s in owned_symbols:
            print(f"  {s['sec']:8s} 0x{s['addr']:06x} size:0x{s['size']:04x} {s['name']}")

        # 2. Outgoing relocations (external references)
        dol_refs = set()
        bases_refs = set()
        internal_refs = set()

        for sec_idx, (start, end) in sec_ranges.items():
            for offset in range(start, end):
                if (sec_idx, offset) in relocs_by_src:
                    for mod_num, target_sec, addend, rtype in relocs_by_src[(sec_idx, offset)]:
                        if mod_num == 0: # DOL
                            target_sec_name = idx_to_sec(target_sec)
                            dol_refs.add((target_sec_name, addend))
                        elif mod_num == 2: # d_basesNP
                            target_sec_name = idx_to_sec(target_sec)
                            # check if target is inside this TU
                            if target_sec in sec_ranges and sec_ranges[target_sec][0] <= addend < sec_ranges[target_sec][1]:
                                internal_refs.add((target_sec_name, addend))
                            else:
                                bases_refs.add((target_sec_name, addend))

        print(f"\nOutgoing DOL references ({len(dol_refs)} distinct addresses):")
        must_not_pin_dol = []
        dol_pinned = []
        dol_unpinned_additions = []

        for sec_name, addr in sorted(dol_refs):
            sym = dol_syms_by_addr.get((sec_name, addr))
            sym_name = sym['name'] if sym else f"dol_{sec_name}_{addr:x}"
            in_syms_txt = sym_name in syms_txt or f"0x{addr:08X}" in syms_txt.values()
            # check if sym is in a landed DOL slice
            is_landed_dol = False
            landed_source = ""
            for s in dol_landed_slices:
                for ssec, srng in s.get('memoryRanges', {}).items():
                    if ssec == sec_name:
                        p = srng.split('-')
                        if int(p[0], 16) <= addr < int(p[1], 16):
                            is_landed_dol = True
                            landed_source = s.get('source')
                            break
            
            if is_landed_dol:
                must_not_pin_dol.append({'name': sym_name, 'addr': f"0x{addr:08X}", 'source': landed_source})
                print(f"  * MUST NOT PIN (in landed DOL {landed_source}): {sym_name} @ 0x{addr:08X}")
            else:
                if sym_name in syms_txt:
                    dol_pinned.append({'name': sym_name, 'addr': f"0x{addr:08X}"})
                    print(f"  P PINNED: {sym_name} @ 0x{addr:08X}")
                else:
                    dol_unpinned_additions.append({'name': sym_name, 'addr': f"0x{addr:08X}"})
                    print(f"  + UNPINNED ADDITION: {sym_name} @ 0x{addr:08X}")

        print(f"\nOutgoing d_basesNP external references ({len(bases_refs)} distinct addresses):")
        must_not_pin_bases = []
        for sec_name, addr in sorted(bases_refs):
            alias_key = f"R_2_{sec_to_idx(sec_name)}_{addr:x}"
            alias_name = alias_db.get(alias_key, "")
            sym = bases_syms_by_addr.get((sec_name, addr))
            sname = sym['name'] if sym else f"bases_{sec_name}_{addr:x}"
            # check if in landed bases slice
            is_landed_bases = False
            landed_source = ""
            for s in bases_landed_slices:
                for ssec, srng in s.get('memoryRanges', {}).items():
                    if ssec == sec_name:
                        p = srng.split('-')
                        if int(p[0], 16) <= addr < int(p[1], 16):
                            is_landed_bases = True
                            landed_source = s.get('source')
                            break
            disp = alias_name if alias_name else sname
            if is_landed_bases:
                must_not_pin_bases.append({'name': disp, 'sec': sec_name, 'offset': f"0x{addr:06X}", 'source': landed_source})
                print(f"  * MUST NOT PIN (in landed REL {landed_source}): {disp} @ {sec_name}:0x{addr:06X}")
            else:
                print(f"  -> External REL ref: {disp} @ {sec_name}:0x{addr:06X} [alias: {alias_key}]")

        landing_kits[tu_name] = {
            'slice_entry': {
                'source': tu_name,
                'memoryRanges': {
                    idx_to_sec(sec_idx): f"0x{start:x}-0x{end:x}"
                    for sec_idx, (start, end) in sec_ranges.items()
                }
            },
            'owned_symbols': owned_symbols,
            'must_not_pin_dol': must_not_pin_dol,
            'must_not_pin_bases': must_not_pin_bases,
            'dol_pinned': dol_pinned,
            'dol_unpinned_additions': dol_unpinned_additions
        }

    with open(os.path.join(ROOT, 'scratch', 'gemini_round13', 'final_landing_kits.json'), 'w') as f:
        json.dump(landing_kits, f, indent=2)

if __name__ == '__main__':
    main()
