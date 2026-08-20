"""Load sweep_raw.json and find small (1-3 word) length mismatches between
draft and target per function -- the mechanical tell for a missing/extra
return value. Does NOT judge the epilogue; that is done separately by
inspecting the target text around the diff.
"""
import json
import os

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"

with open(os.path.join(ROOT, 'wip', 'return_type_sweep', 'sweep_raw.json'),
          encoding='utf-8') as fh:
    data = json.load(fh)

hits = []
matched = 0
unauthored = 0
big_mismatch = 0
total_funcs = 0

for unit, info in data.items():
    if 'error' in info:
        print('UNIT ERROR:', unit, info.get('error'))
        continue
    funcs = info.get('functions', {})
    for name, f in funcs.items():
        total_funcs += 1
        tl = f['target_len']
        dl = f['draft_len']
        if dl is None:
            unauthored += 1
            continue
        delta = dl - tl  # positive: draft longer than target
        if delta == 0:
            matched += 1
            continue
        if 1 <= abs(delta) <= 3:
            hits.append({
                'unit': unit, 'name': name, 'target_file': f['target_file'],
                'target_len': tl, 'draft_len': dl, 'delta': delta,
            })
        else:
            big_mismatch += 1

hits.sort(key=lambda h: (h['unit'], h['name']))

print('=== SUMMARY ===')
print('total functions examined:', total_funcs)
print('exact length match:', matched)
print('unauthored (draft missing entirely):', unauthored)
print('big mismatch (>3 words off):', big_mismatch)
print('SMALL (1-3 word) mismatches:', len(hits))
print()
for h in hits:
    print('%-20s %-45s target=%-4d draft=%-4d delta=%+d  (%s)' % (
        h['unit'], h['name'], h['target_len'], h['draft_len'], h['delta'], h['target_file']))

with open(os.path.join(ROOT, 'wip', 'return_type_sweep', 'small_mismatches.json'),
          'w', encoding='utf-8') as fh:
    json.dump(hits, fh, indent=1)
