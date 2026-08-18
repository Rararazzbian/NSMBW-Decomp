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

    # Let's inspect symbols in .rodata, .data, .bss for d_en_bossNP.rel
    rodata_syms = [s for s in boss_syms if s['sec'] == '.rodata']
    data_syms = [s for s in boss_syms if s['sec'] == '.data']
    bss_syms = [s for s in boss_syms if s['sec'] == '.bss']
    rodata_syms.sort(key=lambda s: s['addr'])
    data_syms.sort(key=lambda s: s['addr'])
    bss_syms.sort(key=lambda s: s['addr'])

    print("Total rodata symbols:", len(rodata_syms))
    print("Total data symbols:", len(data_syms))
    print("Total bss symbols:", len(bss_syms))

    # Let's inspect the exact ranges between TUs in .data, .rodata, .bss
    # We have 26 TUs.
    # In .data, each TU typically starts with string literals/profile and ends with its class vtable!
    # Let's find all vtable symbols in .data
    # A vtable is referenced by constructor/destructor in .text
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

    # For each of the 8 candidate TUs, let's find the exact data, rodata, and bss bracketing
    # Candidate indices: 11, 14, 15, 2, 17, 1, 20, 23
    # Let's inspect candidate 11 (TU 11: 0x29118-0x29efc)
    # What .rodata, .data, .bss are between TU 10 (Bowser) and TU 12 (Demo Kameck)?
    print("\n--- Bracketing Analysis for Candidate 1: TU 11 (Koopa Demo Cage) ---")
    # TU 10 text ends at 0x29118, TU 11 text is 0x29118-0x29efc, TU 12 text starts at 0x29efc
    # ctors: slot 0x28-0x2c (TU 10 is 0x24-0x28, TU 12 is 0x2c-0x30)
    # rodata:
    rodata_11 = [s for s in rodata_syms if 0xc00 <= s['addr'] <= 0xd00]
    for s in rodata_11:
        print(f"  rodata:0x{s['addr']:x} size=0x{s['size']:x} {s['name']}")
    # data:
    data_11 = [s for s in data_syms if 0x9000 <= s['addr'] <= 0xa200]
    for s in data_11:
        print(f"  data:0x{s['addr']:x} size=0x{s['size']:x} {s['name']}")
    # bss:
    bss_11 = [s for s in bss_syms if 0x2600 <= s['addr'] <= 0x2750]
    for s in bss_11:
        print(f"  bss:0x{s['addr']:x} size=0x{s['size']:x} {s['name']}")

if __name__ == '__main__':
    main()
