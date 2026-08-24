import json

s = json.load(open('/opt/NSMBW-Decomp/slices/wiimj2d.json'))
slices = s['slices']
print(json.dumps(slices[0], indent=1)[:400])
for d in slices:
    txt = json.dumps(d)
    if 'spin_child' in txt or 'd_actor' in txt.lower() or 'zoom' in txt:
        print(json.dumps(d, indent=1))
