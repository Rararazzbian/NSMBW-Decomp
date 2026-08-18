import os
import json

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

with open(os.path.join(ROOT, 'slices', 'd_basesNP.json')) as f:
    slices_data = json.load(f)

# Parse existing slice ranges
existing_ranges = {}
for s in slices_data.get('slices', []):
    src = s.get('source')
    for sec, rng_str in s.get('memoryRanges', {}).items():
        parts = rng_str.split('-')
        start, end = int(parts[0], 16), int(parts[1], 16)
        if sec not in existing_ranges:
            existing_ranges[sec] = []
        existing_ranges[sec].append((start, end, src))

proposed_slices = {
    'd_basesNP/bases/d_a_wm_grid.cpp': {
        '.text': (0x164210, 0x164404),
        '.ctors': (0x3e4, 0x3e8),
        '.rodata': (0x88b8, 0x88c8),
        '.data': (0x44c90, 0x44d20),
        '.bss': (0xfdd0, 0xfde0),
    },
    'd_basesNP/bases/d_a_wm_tower.cpp': {
        '.text': (0x1856f0, 0x185b44),
        '.ctors': (0x44c, 0x450),
        '.rodata': (0x9320, 0x9330),
        '.data': (0x48090, 0x48158),
        '.bss': (0x10350, 0x10360),
    }
}

print("=== Overlap and Adjacency Check ===")
for tu, secs in proposed_slices.items():
    print(f"\nChecking TU: {tu}")
    for sec, (p_start, p_end) in secs.items():
        print(f"  Section {sec:8s}: 0x{p_start:06x} - 0x{p_end:06x} (size: 0x{p_end-p_start:x})")
        overlaps = []
        lower_neighbors = []
        upper_neighbors = []
        for e_start, e_end, src in existing_ranges.get(sec, []):
            # Check overlap
            if max(p_start, e_start) < min(p_end, e_end):
                overlaps.append((src, e_start, e_end))
            if e_end <= p_start:
                lower_neighbors.append((e_end, p_start - e_end, src))
            if e_start >= p_end:
                upper_neighbors.append((e_start, e_start - p_end, src))
        
        if overlaps:
            print(f"    ERROR: OVERLAP with {overlaps}")
        else:
            print("    Overlap: NONE (clean)")
        
        if lower_neighbors:
            lower_neighbors.sort(key=lambda x: x[1])
            closest = lower_neighbors[0]
            print(f"    Closest lower landed slice: {closest[2]} at 0x{closest[0]:x} (gap: 0x{closest[1]:x})")
        if upper_neighbors:
            upper_neighbors.sort(key=lambda x: x[1])
            closest = upper_neighbors[0]
            print(f"    Closest upper landed slice: {closest[2]} at 0x{closest[0]:x} (gap: 0x{closest[1]:x})")
