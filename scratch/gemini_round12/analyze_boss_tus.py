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

    # Section sizes
    # .text: size = rel.sections[1].size()
    # .ctors: size = rel.sections[2].size()
    # .dtors: size = rel.sections[3].size()
    # .rodata: size = rel.sections[4].size()
    # .data: size = rel.sections[5].size()
    # .bss: size = rel.bss_size
    print(f".text size: 0x{rel.sections[1].size():x}")
    print(f".ctors size: 0x{rel.sections[2].size():x}")
    print(f".dtors size: 0x{rel.sections[3].size():x}")
    print(f".rodata size: 0x{rel.sections[4].size():x}")
    print(f".data size: 0x{rel.sections[5].size():x}")
    print(f".bss size: 0x{rel.bss_size:x}")

    # Read relocations to map ctors
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

    # Identify TU boundaries
    # Each sinit is at or near the end of its TU's .text section!
    # Let's inspect the split objects in bin/dtkspl/d_en_bossNP/obj/
    split_obj_dir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_en_bossNP', 'obj')
    split_files = os.listdir(split_obj_dir)
    
    print(f"\nSplit objects in {split_obj_dir}: {len(split_files)}")
    text_objs = [f for f in split_files if f.endswith('_text.o')]
    text_objs.sort()
    for o in text_objs:
        print(f"  {o}")

if __name__ == '__main__':
    main()
