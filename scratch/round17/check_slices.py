import json

with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\slices\wiimj2d.json', encoding='utf-8') as fh:
    d = json.load(fh)

print('top-level keys:', list(d.keys())[:10])
slices = d.get('slices')
if slices is None:
    # maybe entries
    for k in ('entries', 'units', 'files'):
        if k in d:
            print('found', k, type(d[k]))
            slices = d[k]
            break

if isinstance(slices, dict):
    items = slices.items()
elif isinstance(slices, list):
    items = [(s.get('name', s.get('file', str(i))), s) for i, s in enumerate(slices)]
else:
    items = []

for name, s in items:
    t = s.get('.text', '')
    if isinstance(t, str) and t:
        try:
            lo, hi = [int(x, 16) for x in t.split('-')]
        except Exception:
            continue
        if lo < 0x8007F800 and hi > 0x8007E000:
            print(name, s)
