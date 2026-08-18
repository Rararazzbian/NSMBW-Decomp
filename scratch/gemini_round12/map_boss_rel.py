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
                stype = 'unknown'
                st_m = re.search(r'type:(\w+)', meta)
                if st_m:
                    stype = st_m.group(1)
                scope = 'local'
                sc_m = re.search(r'scope:(\w+)', meta)
                if sc_m:
                    scope = sc_m.group(1)
                syms.append({'name': name, 'sec': sec, 'addr': addr, 'size': size, 'type': stype, 'scope': scope})
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

    print("=== .ctors mapping in d_en_bossNP.rel ===")
    ctors_slots = []
    for pos in sorted(relocs_by_src.keys()):
        if pos[0] == 2: # .ctors
            for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                ctors_slots.append((pos[1], dst_addend))
                print(f"  ctors 0x{pos[1]:x} -> sinit at .text:0x{dst_addend:x}")

    print(f"\nTotal ctors slots: {len(ctors_slots)}")
    
    # Let's inspect profiles in .data (section 5)
    profiles = []
    for k, v in mod4_aliases.items():
        if k.startswith('R_4_5_') and 'profile' in v.lower():
            offset = int(k.split('_')[3], 16)
            profiles.append((offset, v))
    profiles.sort()
    print(f"\nFound {len(profiles)} profiles in d_en_bossNP.rel:")
    for off, name in profiles:
        print(f"  .data:0x{off:05x} {name}")

    # Let's map translation units by analyzing sinit functions, profiles, and vtables!
    # In .text, let's find all functions
    text_syms = [s for s in boss_syms if s['sec'] == '.text']
    text_syms.sort(key=lambda s: s['addr'])
    print(f"\nTotal .text functions: {len(text_syms)}")
    
    # Check how many are real mangled names vs fn_4_*
    named_fns = [s for s in text_syms if not s['name'].startswith('fn_4_') and not s['name'].startswith('gap_') and not s['name'].startswith('pad_')]
    anon_fns = [s for s in text_syms if s['name'].startswith('fn_4_')]
    print(f"Named functions: {len(named_fns)}, Anonymous functions: {len(anon_fns)}")
    print(f"Named functions list:")
    for s in named_fns:
        print(f"  .text:0x{s['addr']:x} {s['name']}")

    # Let's also check data section objects (vtables, strings, etc.)
    data_syms = [s for s in boss_syms if s['sec'] == '.data']
    data_syms.sort(key=lambda s: s['addr'])
    print(f"\nTotal .data symbols: {len(data_syms)}")
    
    # Save text syms and data syms to JSON for detailed grouping
    with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'boss_symbols_dump.json'), 'w') as f:
        json.dump({
            'ctors_slots': ctors_slots,
            'profiles': profiles,
            'text_syms': text_syms,
            'data_syms': data_syms,
            'rodata_syms': [s for s in boss_syms if s['sec'] == '.rodata'],
            'bss_syms': [s for s in boss_syms if s['sec'] == '.bss']
        }, f, indent=2)

if __name__ == '__main__':
    main()
