import json
import os

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"

with open(os.path.join(ROOT, 'wip', 'return_type_sweep', 'match_raw.json'),
          encoding='utf-8') as fh:
    data = json.load(fh)

hits = []
n_exact = 0
n_no_candidate = 0
n_big = 0
n_examined = 0

for unit, info in data.items():
    if 'error' in info:
        print('UNIT ERROR:', unit, info['error'])
        continue
    for p in info['pairs']:
        n_examined += 1
        if p['match']:
            n_exact += 1
            continue
        if p['draft_len'] is None:
            n_no_candidate += 1
            continue
        d = p['delta']
        if 1 <= abs(d) <= 3:
            hits.append({
                'unit': unit, 'addr': p['addr'], 'target_name': p['name'],
                'target_len': p['target_len'], 'draft_name': p['draft_name'],
                'draft_len': p['draft_len'], 'delta': d,
                'diffcount': p['diffcount'],
            })
        else:
            n_big += 1

hits.sort(key=lambda h: (h['unit'], h['addr']))

print('=== SUMMARY ===')
print('functions examined:', n_examined)
print('exact / tail-blr match:', n_exact)
print('no unused draft candidate left:', n_no_candidate)
print('mismatch >3 words:', n_big)
print('SMALL (1-3 word) mismatches:', len(hits))
print()
for h in hits:
    print('%-20s %#010x  target=%-22s (%d)  draft~%-22s (%d)  delta=%+d  diffcount=%s' % (
        h['unit'], h['addr'], h['target_name'], h['target_len'],
        h['draft_name'], h['draft_len'], h['delta'], h['diffcount']))

with open(os.path.join(ROOT, 'wip', 'return_type_sweep', 'small_mismatches.json'),
          'w', encoding='utf-8') as fh:
    json.dump(hits, fh, indent=1)
