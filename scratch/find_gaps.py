import os
import re
import sys

sys.path.insert(0, os.path.abspath('.'))

# 1. Parse dtk_splits_wiimj2d.txt for landed .text ranges
landed_ranges = []
with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r', encoding='utf-8') as f:
    current_tu = None
    for line in f:
        line = line.rstrip()
        if not line or line.startswith('Sections:'):
            continue
        if not line.startswith('\t') and line.endswith(':'):
            current_tu = line[:-1]
        elif line.startswith('\t') and current_tu is not None:
            parts = line.strip().split()
            if parts[0] == '.text':
                start = int(parts[1].split(':')[1], 16)
                end = int(parts[2].split(':')[1], 16)
                landed_ranges.append((start, end, current_tu))

landed_ranges.sort()
print(f"Total landed .text ranges in wiimj2d: {len(landed_ranges)}")

# 2. Find all gaps between landed ranges
# The .text section runs from 0x80006780 to ~0x802EB000
gaps = []
prev_end = 0x80006780

for start, end, name in landed_ranges:
    if start > prev_end:
        gaps.append((prev_end, start))
    prev_end = max(prev_end, end)

print(f"Total gaps in .text: {len(gaps)}")
for g_start, g_end in gaps:
    size = g_end - g_start
    print(f"Gap: 0x{g_start:08X} - 0x{g_end:08X} (size: 0x{size:X} = {size} bytes)")
