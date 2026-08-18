import json
import re

with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)

meta_sections = slice_data['meta']['sections']
sec_bases = {sname: int(sinfo['addr'], 16) for sname, sinfo in meta_sections.items()}

# Let's inspect all parsed slices in wiimj2d.json
slices = slice_data['slices']

print("Total slices in wiimj2d.json:", len(slices))

def find_neighbors(sec_name, start_offs, end_offs):
    # returns slices that overlap, slice immediately before, slice immediately after
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

            # check overlap
            if max(start_offs, s_val) < min(end_offs, e_val):
                overlaps.append((sl.get('source', sl.get('sliceName')), s_val, e_val))

            # check before
            if e_val <= start_offs:
                if e_val > max_before_end:
                    max_before_end = e_val
                    before = (sl.get('source', sl.get('sliceName')), s_val, e_val)

            # check after
            if s_val >= end_offs:
                if s_val < min_after_start:
                    min_after_start = s_val
                    after = (sl.get('source', sl.get('sliceName')), s_val, e_val)

    return overlaps, before, after

print("\n==========================================")
print("=== AUDIT FOR d_a_en_coin_main.cpp ===")
print("==========================================")

# Coin main ranges from round 9 corrected:
# .text: 0x800272F0 - 0x800281C0
# .ctors: 0x802EDD18 - 0x802EDD1C
# .rodata: 0x802EE750 - 0x802EE7F0 (or 0x802EE810? let's check size)
# .data: 0x80303078 - 0x80303368 (or 0x49D8 - 0x4CC8?)
# .bss: 0x803530E8 - 0x80353120 (0x1768 - 0x17A0?)
# .sdata2: 0x8042B630 - 0x8042B638 (0x2D0 - 0x2D8?)

coin_sections = {
    '.text': (0x800272F0, 0x800281C0),
    '.ctors': (0x802EDD18, 0x802EDD1C),
    '.rodata': (0x802EE750, 0x802EE7F0), # wait, let's verify exact end
    '.data': (0x80303078, 0x80303368),
    '.bss': (0x803530E8, 0x80353120),
    '.sdata2': (0x8042B630, 0x8042B638)
}

for sec, (v_start, v_end) in coin_sections.items():
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

print("\n==========================================")
print("=== AUDIT FOR m_pad.cpp ===")
print("==========================================")

# Let's check m_pad virtual ranges from symbols:
# .text: 0x8016F330 .. 0x8016F860
# .ctors: 0x802EDE28 .. 0x802EDE2C (let's check!)
# .rodata: none?
# .data: none?
# .bss: 0x80377F88 .. 0x803780C8
# .sbss: 0x8042A740 .. 0x8042A760

# Let's check exact symbols and bounds for m_pad!
