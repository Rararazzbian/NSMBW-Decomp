import json

with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)

rodata_base = 0x802EDFE0
slices = slice_data['slices']

for i, s in enumerate(slices):
    mr = s.get('memoryRanges', {})
    if '.rodata' in mr:
        r = mr['.rodata']
        parts = r.split('-')
        start = int(parts[0], 16)
        end = int(parts[1], 16)
        print(f"[{i:3d}] {s['source']:<40} rodata: 0x{start:04x}-0x{end:04x} (VA 0x{rodata_base+start:08x}-0x{rodata_base+end:08x})")

