import json, os

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"

with open(os.path.join(ROOT, 'slices', 'wiimj2d.json'), 'r') as f:
    data = json.load(f)

meta = data['meta']
sections = meta['sections']

def parse_range(r_str):
    start, end = r_str.split('-')
    return int(start, 16), int(end, 16)

our_ranges = {
    '.text': parse_range('0xc8170-0xc8580'),
    '.rodata': parse_range('0x3480-0x3490'),
    '.data': parse_range('0x19628-0x19638'),
    '.sbss': parse_range('0x3f0-0x3f8'),
    '.sdata2': parse_range('0x1930-0x1938'),
}

print("=== Checking for Overlaps with 144 Banked Slices ===")
collisions = 0
for slice_info in data['slices']:
    src = slice_info['source']
    mem = slice_info['memoryRanges']
    for sec, (our_start, our_end) in our_ranges.items():
        if sec in mem:
            s_start, s_end = parse_range(mem[sec])
            # Check overlap: [our_start, our_end) and [s_start, s_end)
            if not (our_end <= s_start or our_start >= s_end):
                print(f"COLLISION in {sec} with {src}: slice {mem[sec]} vs ours (0x{our_start:x}-0x{our_end:x})")
                collisions += 1

if collisions == 0:
    print("ALL CLEAR: 0 collisions across all 144 banked slices in all sections!")
