"""Check which landed slice owns .data around 0x44A68-0x44B00."""
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
d = json.load(open(os.path.join(ROOT, 'slices', 'd_basesNP.json')))

slices = d if isinstance(d, list) else d.get('slices', [])
print('total slices:', len(slices))

for s in slices:
    src = s.get('source', '')
    mr = s.get('memoryRanges', {})
    data = mr.get('.data', '')
    if not data:
        continue
    lo, hi = (int(x, 16) for x in data.split('-'))
    # Print any .data range overlapping the interesting window.
    if lo < 0x44C00 and hi > 0x44700:
        print('%-50s %s' % (src, data))
