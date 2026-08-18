import json, re

with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)
meta_sections = slice_data['meta']['sections']
sec_bases = {sname: int(sinfo['addr'], 16) for sname, sinfo in meta_sections.items()}
slices = slice_data['slices']

def find_neighbors(sec_name, start_offs, end_offs):
    overlaps = []
    before = None
    after = None
    max_before_end = -1
    min_after_start = 0x7FFFFFFF

    for sl in slices:
        mr = sl.get('memoryRanges', {})
        if sec_name in mr:
            r = mr[sec_name]
            s_str, e_str = r.split('-')
            s_val = int(s_str, 16)
            e_val = int(e_str, 16)

            if max(start_offs, s_val) < min(end_offs, e_val):
                overlaps.append((sl.get('source', sl.get('sliceName')), s_val, e_val))

            if e_val <= start_offs:
                if e_val > max_before_end:
                    max_before_end = e_val
                    before = (sl.get('source', sl.get('sliceName')), s_val, e_val)

            if s_val >= end_offs:
                if s_val < min_after_start:
                    min_after_start = s_val
                    after = (sl.get('source', sl.get('sliceName')), s_val, e_val)

    return overlaps, before, after

pad_sections = {
    '.text': (0x8016F330, 0x8016F880),
    '.ctors': (0x802EDEFC, 0x802EDF00),
    '.bss': (0x80377F88, 0x803780C8),
    '.sbss': (0x8042A740, 0x8042A760)
}

print("=== AUDIT FOR m_pad.cpp ===")
for sec, (v_start, v_end) in pad_sections.items():
    base = sec_bases[sec]
    start_offs = v_start - base
    end_offs = v_end - base
    overlaps, before, after = find_neighbors(sec, start_offs, end_offs)
    print(f"Section {sec}: virt={hex(v_start)}..{hex(v_end)} (base={hex(base)}) -> offs={hex(start_offs)}-{hex(end_offs)}")
    print(f"  Overlaps: {overlaps}")
    print(f"  Before:   {before}")
    print(f"  After:    {after}")
    if before:
        print(f"    Adjacent before? {before[2] == start_offs} (gap = {hex(start_offs - before[2])})")
    if after:
        print(f"    Adjacent after?  {after[1] == end_offs} (gap = {hex(after[1] - end_offs)})")

# Check if m_pad has any .rodata, .data, .sdata, .sdata2
for sec in ['.rodata', '.data', '.sdata', '.sdata2']:
    print(f"Checking empty section {sec}...")
    # what is the gap between neighbours in this section around m_pad?
