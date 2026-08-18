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
    alias_db = {}
    with open(os.path.join(ROOT, 'alias_db.txt'), 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#') and '=' in line:
                k, v = line.split('=', 1)
                alias_db[k.strip()] = v.strip()

    syms_txt = {}
    with open(os.path.join(ROOT, 'syms.txt'), 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#') and '=' in line:
                k, v = line.split('=', 1)
                syms_txt[k.strip()] = int(v.strip(), 16)

    with open(os.path.join(ROOT, 'slices', 'wiimj2d.json')) as f:
        dol_slices_data = json.load(f)
    dol_landed_slices = [s for s in dol_slices_data['slices'] if s.get('source') and not s.get('nonMatching')]
    dol_meta_sections = dol_slices_data['meta']['sections']

    with open(os.path.join(ROOT, 'slices', 'd_basesNP.json')) as f:
        bases_slices_data = json.load(f)
    bases_landed_slices = [s for s in bases_slices_data['slices'] if s.get('source') and not s.get('nonMatching')]

    dol_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'))
    bases_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt'))

    with open(os.path.join(ROOT, 'original', 'd_basesNP.rel'), 'rb') as f:
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

    targets = {
        'd_a_wm_grid.cpp': {
            1: (0x164230, 0x164430),
            2: (0x3e4, 0x3e8),
            4: (0x88b8, 0x88d0),
            5: (0x44cb4, 0x44d54),
            6: (0xfdd0, 0xfde0),
        },
        'd_a_wm_tower.cpp': {
            1: (0x185710, 0x185b70),
            2: (0x44c, 0x450),
            4: (0x9488, 0x94a0),
            5: (0x480b4, 0x4818c),
            6: (0x10a98, 0x10aa8),
        },
        'd_a_wm_smallcloud.cpp': {
            1: (0x1797e0, 0x179ff0),
            2: (0x430, 0x434),
            4: (0x8f58, 0x8fa0),
            5: (0x4728c, 0x47484),
            6: (0x10130, 0x10140),
        },
        'd_a_wm_kinoko_base.cpp': {
            1: (0x16b2d0, 0x16bda0),
            2: (0x3fc, 0x400),
            4: (0x8ac8, 0x8af0),
            5: (0x458c0, 0x45a90),
            6: (0xfe80, 0xfe90),
        }
    }

    sec_names = {1: '.text', 2: '.ctors', 3: '.dtors', 4: '.rodata', 5: '.data', 6: '.bss'}

    def is_dol_addr_landed(sec_name, addr):
        for s in dol_landed_slices:
            if sec_name in s['memoryRanges']:
                r_str = s['memoryRanges'][sec_name]
                start_off, end_off = [int(x, 16) for x in r_str.split('-')]
                base_addr = int(dol_meta_sections[sec_name]['addr'], 16)
                if base_addr + start_off <= addr < base_addr + end_off:
                    return s['source']
        return None

    def is_bases_addr_landed(sec_name, addr):
        for s in bases_landed_slices:
            if sec_name in s['memoryRanges']:
                r_str = s['memoryRanges'][sec_name]
                start_off, end_off = [int(x, 16) for x in r_str.split('-')]
                if start_off <= addr < end_off:
                    return s['source']
        return None

    out_data = {}

    for tu_name, ranges in targets.items():
        referenced_symbols = set()
        for sec_idx, (s_start, s_end) in ranges.items():
            for pos in sorted(relocs_by_src.keys()):
                if pos[0] == sec_idx and s_start <= pos[1] < s_end:
                    for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                        referenced_symbols.add((dst_mod, dst_sec, dst_addend))

        mod0_refs = []
        mod2_refs = []

        for dst_mod, dst_sec, dst_addend in sorted(referenced_symbols):
            if dst_mod == 0:
                sym_match = None
                for s in dol_syms:
                    if s['addr'] <= dst_addend < s['addr'] + max(s['size'], 1):
                        sym_match = s
                        break
                if not sym_match:
                    for s in dol_syms:
                        if s['addr'] == dst_addend:
                            sym_match = s
                            break
                mod0_refs.append((dst_sec, dst_addend, sym_match))
            elif dst_mod == 2:
                sec_name = sec_names.get(dst_sec, f"sec_{dst_sec}")
                sym_match = None
                for s in bases_syms:
                    if s['sec'] == sec_name and s['addr'] <= dst_addend < s['addr'] + max(s['size'], 1):
                        sym_match = s
                        break
                alias_name = alias_db.get(f"R_2_{dst_sec}_{dst_addend:x}", "")
                mod2_refs.append((dst_sec, dst_addend, sym_match, alias_name))

        must_not_pin_dol = []
        unlanded_dol_pinned = []
        unlanded_dol_unpinned = []

        for dst_sec, dst_addend, sym in mod0_refs:
            sym_name = sym['name'] if sym else f"UNK_0x{dst_addend:08X}"
            sec_name = sym['sec'] if sym else "unknown"
            landed_by = is_dol_addr_landed(sec_name, dst_addend)
            is_pinned = (sym_name in syms_txt)
            if landed_by:
                must_not_pin_dol.append({'name': sym_name, 'addr': f"0x{dst_addend:08X}", 'source': landed_by})
            elif is_pinned:
                unlanded_dol_pinned.append({'name': sym_name, 'addr': f"0x{dst_addend:08X}"})
            else:
                unlanded_dol_unpinned.append({'name': sym_name, 'addr': f"0x{dst_addend:08X}"})

        must_not_pin_bases = []
        other_bases_refs = []
        for dst_sec, dst_addend, sym, alias in mod2_refs:
            sec_name = sec_names.get(dst_sec, f"sec_{dst_sec}")
            is_internal = False
            if dst_sec in ranges:
                if ranges[dst_sec][0] <= dst_addend < ranges[dst_sec][1]:
                    is_internal = True
            if is_internal:
                continue
            landed_by = is_bases_addr_landed(sec_name, dst_addend)
            sym_name = alias if alias else (sym['name'] if sym else f"R_2_{dst_sec}_{dst_addend:x}")
            if landed_by:
                must_not_pin_bases.append({'name': sym_name, 'sec': sec_name, 'offset': f"0x{dst_addend:x}", 'source': landed_by})
            else:
                other_bases_refs.append({'name': sym_name, 'sec': sec_name, 'offset': f"0x{dst_addend:x}"})

        # Check if any symbols within this TU's ranges are in syms.txt (Removals)
        # Note: In syms.txt, all symbols have virtual addresses starting with 0x80xxxxxx (DOL).
        # Are any symbols from d_basesNP in syms.txt?
        removals = []
        # Check all symbols in bases_syms within ranges
        for sec_idx, (s_start, s_end) in ranges.items():
            sec_name = sec_names.get(sec_idx, f"sec_{sec_idx}")
            for s in bases_syms:
                if s['sec'] == sec_name and s_start <= s['addr'] < s_end:
                    if s['name'] in syms_txt:
                        removals.append({'name': s['name'], 'addr': f"0x{syms_txt[s['name']]:08X}"})

        out_data[tu_name] = {
            'must_not_pin_dol': must_not_pin_dol,
            'must_not_pin_bases': must_not_pin_bases,
            'dol_pinned': unlanded_dol_pinned,
            'dol_unpinned_additions': unlanded_dol_unpinned,
            'removals': removals,
            'other_bases_refs': other_bases_refs
        }

    with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'landing_kits_data.json'), 'w') as f:
        json.dump(out_data, f, indent=2)

    print("Wrote landing_kits_data.json successfully.")

if __name__ == '__main__':
    main()
