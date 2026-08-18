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

    print(f"Total ctors slots: {len(ctors_slots)}")
    for c_off, sinit_addr in ctors_slots:
        print(f"  ctors 0x{c_off:02x} -> sinit 0x{sinit_addr:05x}")

    # Inspect profiles in .data
    profiles = []
    for k, v in mod4_aliases.items():
        if k.startswith('R_4_5_') and 'profile' in v.lower():
            offset = int(k.split('_')[3], 16)
            profiles.append((offset, v))
    profiles.sort()

    # In d_en_bossNP, let's see what each profile points to!
    # A Profile struct has factory function pointer, etc.
    # Let's see what relocations exist in .data at each profile offset
    print(f"\n--- Profiles and their factory / classInit functions ---")
    for p_off, p_name in profiles:
        # profile struct is typically 0xC or 0x14 bytes
        # relocations at p_off, p_off+4, etc.
        p_relocs = [relocs_by_src[(5, p_off + i)] for i in range(0, 0x14, 4) if (5, p_off + i) in relocs_by_src]
        print(f"  Profile {p_name} at .data:0x{p_off:05x}:")
        for i in range(0, 0x14, 4):
            if (5, p_off + i) in relocs_by_src:
                for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[(5, p_off + i)]:
                    print(f"    +0x{i:02x} -> Mod {dst_mod}, Sec {dst_sec}, Addr 0x{dst_addend:x}")

    # Let's inspect split object files in bin/dtkspl/d_en_bossNP/obj/
    split_dir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_en_bossNP', 'obj')
    files = sorted(os.listdir(split_dir))
    print(f"\nFiles in {split_dir}:")
    for f in files:
        if f.endswith('.o'):
            print(f"  {f}")

if __name__ == '__main__':
    main()
