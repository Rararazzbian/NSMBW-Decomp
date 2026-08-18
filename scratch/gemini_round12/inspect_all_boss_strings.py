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
    dol_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'))
    dol_by_addr = {s['addr']: s for s in dol_syms}

    with open(os.path.join(ROOT, 'original', 'd_en_bossNP.rel'), 'rb') as f:
        rel = Rel(4, file=f)

    data_raw = rel.sections[5].get_data()
    rodata_raw = rel.sections[4].get_data()

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

    split_starts = [
        0x110, 0x45dc, 0x666c, 0x9bc4, 0xde30, 0x11738, 0x156d0, 0x1807c,
        0x1c12c, 0x2102c, 0x29118, 0x29efc, 0x2beb4, 0x2dff8, 0x2fbf4,
        0x31e94, 0x34800, 0x377e8, 0x3d038, 0x42798, 0x4635c, 0x4a35c,
        0x4ee48, 0x50744, 0x51bc4, 0x55c94, 0x5702c
    ]

    for i in range(26):
        t_start = split_starts[i]
        t_end = split_starts[i+1]
        
        # Find all strings in data / rodata referenced by this TU
        strings_found = set()
        dol_calls = set()
        for pos in relocs_by_src:
            if pos[0] == 1 and t_start <= pos[1] < t_end:
                for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                    if dst_mod == 4 and dst_sec == 5:
                        # string in data?
                        # check bytes at dst_addend in data_raw
                        if dst_addend < len(data_raw):
                            end_null = data_raw.find(b'\0', dst_addend)
                            if end_null != -1 and end_null - dst_addend < 64:
                                s = data_raw[dst_addend:end_null].decode('ascii', errors='ignore')
                                if len(s) >= 3 and s.isprintable():
                                    strings_found.add(s)
                    elif dst_mod == 4 and dst_sec == 4:
                        if dst_addend < len(rodata_raw):
                            end_null = rodata_raw.find(b'\0', dst_addend)
                            if end_null != -1 and end_null - dst_addend < 64:
                                s = rodata_raw[dst_addend:end_null].decode('ascii', errors='ignore')
                                if len(s) >= 3 and s.isprintable():
                                    strings_found.add(s)
                    elif dst_mod == 0:
                        if dst_addend in dol_by_addr:
                            dol_calls.add(dol_by_addr[dst_addend]['name'])

        print(f"\nTU {i+1:2d} (0x{t_start:05x}-0x{t_end:05x}):")
        print(f"  Strings: {sorted(list(strings_found))[:8]}")
        print(f"  DOL calls sample: {sorted(list(dol_calls))[:5]}")

if __name__ == '__main__':
    main()
