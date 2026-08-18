import json
import sys

with open('slices/wiimj2d.json') as f:
    slices_data = json.load(f)

print('=== META SECTIONS ===')
for sec, info in slices_data['meta']['sections'].items():
    print(sec, info)
