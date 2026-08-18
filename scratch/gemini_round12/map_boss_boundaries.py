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

    with open(os.path.join(ROOT, 'original', 'd_en_bossNP.rel'), 'rb') as f:
        rel = Rel(4, file=f)

    # Read relocations
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

    # Let's inspect string literals and references in each TU to identify TU source names and roles!
    # Let's find all strings in .rodata / .data
    # In .rodata and .data, string literals like "d_a_en_boss_...", "d_en_boss_...", actor names, etc.
    data_raw = rel.sections[5].get_data()
    rodata_raw = rel.sections[4].get_data()

    # Find string literals in data and rodata
    def extract_strings(data, base_offset):
        strings = {}
        for m in re.finditer(b'([a-zA-Z0-9_./-]{3,})', data):
            s = m.group(1).decode('ascii', errors='ignore')
            strings[base_offset + m.start()] = s
        return strings

    data_strings = extract_strings(data_raw, 0)
    rodata_strings = extract_strings(rodata_raw, 0)

    print("=== Extracting TU details from d_en_bossNP.rel ===")
    
    # Let's map the 26 ctors / TUs
    # Each TU i has:
    # ctors: slot i*4
    # sinit at ctors_slots[i][1]
    
    # Let's find text boundaries for each TU:
    # TU 0: global_destructor_chain (0x70 - 0x110)
    # TU 1: 0x110 - sinit 0x3790 (ends at 0x37c0 or similar)
    # Let's check text function ranges
    text_syms = [s for s in boss_syms if s['sec'] == '.text']
    text_syms.sort(key=lambda s: s['addr'])

    # Let's list sinit addresses
    sinits = [s[1] for s in ctors_slots]
    print(f"Sinits: {[hex(s) for s in sinits]}")

    # Let's find for each sinit where the TU starts and ends
    # In .text, each sinit is followed by __dt static helper functions if any, then the next TU starts 16-aligned!
    tu_text_ranges = []
    
    # Runtime TU 0 is 0x0 - 0x70 (rel_init.cpp)
    # Runtime TU 1 is 0x70 - 0x110 (global_destructor_chain.c)
    
    cur_start = 0x110
    for idx, sinit in enumerate(sinits):
        # Find sinit sym
        s_sym = [s for s in text_syms if s['addr'] == sinit][0]
        # Functions after sinit before next TU
        s_idx = text_syms.index(s_sym)
        # Scan forward until the next function that is either start of next TU or alignment
        # In MWCC, after sinit there might be static destructors (e.g. __dt__...), then next TU starts.
        # Let's see: if idx < len(sinits) - 1: next sinit is sinits[idx+1]
        # The next TU typically starts after the last function belonging to current TU.
        # Let's inspect the functions between sinit and next sinit!
        next_sinit = sinits[idx+1] if idx+1 < len(sinits) else 0x5702c
        
        # Let's look at functions between s_idx and next_sinit
        sub_fns = [s for s in text_syms if cur_start <= s['addr'] < next_sinit]
        # sinit is inside sub_fns. Functions after sinit:
        sinit_pos = [i for i, f in enumerate(sub_fns) if f['addr'] == sinit][0]
        after_sinit = sub_fns[sinit_pos:]
        # Usually sinit is either the last function or followed by 1-2 static destructors (size 0x1c-0x3c)
        # Let's find where the next TU starts by looking at split object boundaries or 16-byte alignment / factory function!
        # The split objects auto_00_* tell us the exact TU start boundaries!
        tu_text_ranges.append((cur_start, sinit, after_sinit))

    # Print summary of split objects in bin/dtkspl/d_en_bossNP/obj/
    split_dir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_en_bossNP', 'obj')
    split_files = [f for f in os.listdir(split_dir) if f.startswith('auto_00_') and f.endswith('_text.o')]
    split_files.sort()
    split_starts = [int(f.split('_')[2], 16) for f in split_files]
    print(f"Split starts from dtk: {[hex(x) for x in split_starts]}")

if __name__ == '__main__':
    main()
