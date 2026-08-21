import json

with open('slices/wiimj2d.json') as f:
    data = json.load(f)

slices = data.get('slices', [])
for i in range(30, 55):
    if i < len(slices):
        s = slices[i]
        print(f"[{i:3d}] {s.get('source'):<40} text={s.get('memoryRanges', {}).get('.text', 'NONE'):<20} ctors={s.get('memoryRanges', {}).get('.ctors', 'NONE'):<12}")
