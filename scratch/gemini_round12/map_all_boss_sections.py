import os
import sys
import json
import re
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from relfile import Rel

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
    with open(os.path.join(ROOT, 'original', 'd_en_bossNP.rel'), 'rb') as f:
        rel = Rel(4, file=f)

    # Map relocations by source (sec, pos) -> [(dst_mod, dst_sec, dst_addend, type)]
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

    # Split starts for .text:
    split_starts = [
        0x110, 0x45dc, 0x666c, 0x9bc4, 0xde30, 0x11738, 0x156d0, 0x1807c,
        0x1c12c, 0x2102c, 0x29118, 0x29efc, 0x2beb4, 0x2dff8, 0x2fbf4,
        0x31e94, 0x34800, 0x377e8, 0x3d038, 0x42798, 0x4635c, 0x4a35c,
        0x4ee48, 0x50744, 0x51bc4, 0x55c94, 0x5702c
    ]

    # Let's inspect .data, .rodata, .bss symbols for each TU
    # For each TU, let's find:
    # 1. ctors: i*4 to (i+1)*4
    # 2. references from its text range to .data, .rodata, .bss
    # 3. relocations from .data / .rodata pointing into its text range (e.g. vtables, function pointers)
    
    rodata_syms = [s for s in boss_syms if s['sec'] == '.rodata']
    data_syms = [s for s in boss_syms if s['sec'] == '.data']
    bss_syms = [s for s in boss_syms if s['sec'] == '.bss']

    tu_sections = []

    for i in range(26):
        t_start = split_starts[i]
        t_end = split_starts[i+1]
        c_start = i * 4
        c_end = (i + 1) * 4

        # Find all data objects referenced by or pointing to this TU's text
        # Data relocations pointing into this TU's text range:
        data_owned = set()
        for pos in relocs_by_src:
            if pos[0] == 5: # .data
                for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                    if dst_mod == 4 and dst_sec == 1 and t_start <= dst_addend < t_end:
                        data_owned.add(pos[1])
            elif pos[0] == 1 and t_start <= pos[1] < t_end:
                for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                    if dst_mod == 4 and dst_sec == 5:
                        data_owned.add(dst_addend)
                    elif dst_mod == 4 and dst_sec == 6:
                        # bss ref
                        pass

        # Rodata references from text:
        rodata_owned = set()
        for pos in relocs_by_src:
            if pos[0] == 1 and t_start <= pos[1] < t_end:
                for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                    if dst_mod == 4 and dst_sec == 4:
                        rodata_owned.add(dst_addend)

        # Bss references from text / data:
        bss_owned = set()
        for pos in relocs_by_src:
            if (pos[0] == 1 and t_start <= pos[1] < t_end) or (pos[0] == 5 and pos[1] in data_owned):
                for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                    if dst_mod == 4 and dst_sec == 6:
                        bss_owned.add(dst_addend)

        tu_sections.append({
            'tu_index': i + 1,
            'text': (t_start, t_end),
            'ctors': (c_start, c_end),
            'data_refs_count': len(data_owned),
            'data_bounds': (min(data_owned) if data_owned else 0, max(data_owned) if data_owned else 0),
            'rodata_refs_count': len(rodata_owned),
            'rodata_bounds': (min(rodata_owned) if rodata_owned else 0, max(rodata_owned) if rodata_owned else 0),
            'bss_refs_count': len(bss_owned),
            'bss_bounds': (min(bss_owned) if bss_owned else 0, max(bss_owned) if bss_owned else 0),
        })

    for s in tu_sections:
        print(f"TU {s['tu_index']:2d} | .text: 0x{s['text'][0]:05x}-0x{s['text'][1]:05x} | .ctors: 0x{s['ctors'][0]:02x}-0x{s['ctors'][1]:02x} | .rodata: 0x{s['rodata_bounds'][0]:04x}-0x{s['rodata_bounds'][1]:04x} | .data: 0x{s['data_bounds'][0]:05x}-0x{s['data_bounds'][1]:05x} | .bss: 0x{s['bss_bounds'][0]:04x}-0x{s['bss_bounds'][1]:04x}")

if __name__ == '__main__':
    main()
