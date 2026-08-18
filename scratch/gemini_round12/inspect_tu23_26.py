import os
import sys
import json
import re
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from relfile import Rel, RelSection, RelRelocation

def main():
    with open(os.path.join(ROOT, 'original', 'd_en_bossNP.rel'), 'rb') as f:
        rel = Rel(4, file=f)

    data_raw = rel.sections[5].get_data()
    rodata_raw = rel.sections[4].get_data()

    def get_strings(data):
        res = []
        for m in re.finditer(b'([a-zA-Z0-9_./-]{3,})', data):
            s = m.group(1).decode('ascii', errors='ignore')
            res.append((m.start(), s))
        return res

    data_strings = get_strings(data_raw)
    rodata_strings = get_strings(rodata_raw)

    # Let's inspect relocations in .text for TUs 23, 24, 25, 26
    # TU 23: 0x4ee48 - 0x50744
    # TU 24: 0x50744 - 0x51bc4
    # TU 25: 0x51bc4 - 0x55c94
    # TU 26: 0x55c94 - 0x5702c
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

    ranges = {
        'TU 23 (0x4ee48-0x50744)': (0x4ee48, 0x50744),
        'TU 24 (0x50744-0x51bc4)': (0x50744, 0x51bc4),
        'TU 25 (0x51bc4-0x55c94)': (0x51bc4, 0x55c94),
        'TU 26 (0x55c94-0x5702c)': (0x55c94, 0x5702c),
    }

    for name, (start, end) in ranges.items():
        print(f"\n======================================")
        print(f"=== {name} ===")
        print(f"======================================")
        data_refs = set()
        rodata_refs = set()
        for pos in relocs_by_src:
            if pos[0] == 1 and start <= pos[1] < end:
                for dst_mod, dst_sec, dst_addend, r_type in relocs_by_src[pos]:
                    if dst_mod == 4 and dst_sec == 5:
                        data_refs.add(dst_addend)
                    elif dst_mod == 4 and dst_sec == 4:
                        rodata_refs.add(dst_addend)
        print(f"Referenced Data Offsets: {[hex(x) for x in sorted(data_refs)[:10]]}")
        print(f"Referenced Rodata Offsets: {[hex(x) for x in sorted(rodata_refs)[:10]]}")
        # Find string matches near these offsets
        print("Strings in data referenced:")
        for doff in sorted(data_refs):
            for str_off, s in data_strings:
                if str_off <= doff < str_off + len(s) + 4:
                    print(f"  data:0x{doff:x} -> '{s}'")
        print("Strings in rodata referenced:")
        for roff in sorted(rodata_refs):
            for str_off, s in rodata_strings:
                if str_off <= roff < str_off + len(s) + 4:
                    print(f"  rodata:0x{roff:x} -> '{s}'")

if __name__ == '__main__':
    main()
