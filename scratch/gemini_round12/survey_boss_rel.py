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
    print("=== Surveying d_en_bossNP.rel ===")
    
    # 1. Load symbols
    boss_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'd_en_bossNP_symbols.txt'))
    print(f"Total symbols in d_en_bossNP_symbols.txt: {len(boss_syms)}")
    
    # 2. Load alias_db.txt
    alias_db = {}
    with open(os.path.join(ROOT, 'alias_db.txt'), 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#') and '=' in line:
                k, v = line.split('=', 1)
                alias_db[k.strip()] = v.strip()

    # Find all aliases for module 4 (d_en_bossNP)
    mod4_aliases = {k: v for k, v in alias_db.items() if k.startswith('R_4_')}
    print(f"Module 4 aliases in alias_db.txt: {len(mod4_aliases)}")
    for k, v in sorted(mod4_aliases.items()):
        print(f"  {k} = {v}")

    # 3. Read original/d_en_bossNP.rel
    with open(os.path.join(ROOT, 'original', 'd_en_bossNP.rel'), 'rb') as f:
        rel = Rel(4, file=f)

    print("\nd_en_bossNP.rel section sizes:")
    for idx, sec in enumerate(rel.sections):
        if sec:
            print(f"  Sec {idx} ({sec.name}): size=0x{sec.size():x}, align={sec.alignment}")

    # 4. Map .ctors entries
    # In .ctors, each 4-byte entry is a pointer to __sinit_*
    # Let's find all relocations in .ctors
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

    print("\n--- .ctors entries in d_en_bossNP.rel ---")
    ctors_entries = []
    # .ctors is section 2
    for pos in sorted(relocs_by_src.keys()):
        if pos[0] == 2:
            targets = relocs_by_src[pos]
            for dst_mod, dst_sec, dst_addend, r_type in targets:
                print(f"  ctors offset 0x{pos[1]:x} -> Mod {dst_mod}, Sec {dst_sec}, Addr 0x{dst_addend:x}")
                ctors_entries.append((pos[1], dst_addend))

    print(f"Total .ctors entries: {len(ctors_entries)}")

if __name__ == '__main__':
    main()
