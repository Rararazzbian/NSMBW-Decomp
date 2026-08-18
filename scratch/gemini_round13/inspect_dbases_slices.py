import os
import sys
import json
import re

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

# Load d_basesNP slice file if exists or inspect dtk_splits_d_basesNP.txt and symbols
splits_file = os.path.join(ROOT, 'bin', 'dtk', 'dtk_splits_d_basesNP.txt')
syms_file = os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt')
slices_file = os.path.join(ROOT, 'slices', 'd_basesNP.json')

with open(slices_file) as f:
    slices_data = json.load(f)

print("d_basesNP slices sections meta:")
for sec_name, meta in slices_data.get('meta', {}).get('sections', {}).items():
    print(f"  {sec_name:10s} base: 0x{meta.get('base', 0):08x} size: 0x{meta.get('size', 0):08x}")

# Let's inspect existing landed slices in d_basesNP.json
print("\nExisting landed slices in d_basesNP.json:")
for slice_entry in slices_data.get('slices', []):
    name = slice_entry.get('sliceName', '')
    secs = slice_entry.get('sections', {})
    print(f"\nSlice: {name}")
    for sec_name, rng in secs.items():
        print(f"  {sec_name:10s}: 0x{rng[0]:08x} - 0x{rng[1]:08x} (size: 0x{rng[1]-rng[0]:x})")
