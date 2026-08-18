import os
import json

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'landing_kits_data.json')) as f:
    data = json.load(f)

for tu, info in data.items():
    print(f"\n======================================")
    print(f"=== {tu} ===")
    print(f"======================================")
    print(f"Removals from syms.txt ({len(info['removals'])}):")
    for r in info['removals']:
        print(f"  - {r['name']} = {r['addr']}")
    print(f"Additions to syms.txt ({len(info['dol_unpinned_additions'])}):")
    for a in info['dol_unpinned_additions']:
        print(f"  + {a['name']} = {a['addr']}")
    print(f"DOL Must-Not-Pin ({len(info['must_not_pin_dol'])}):")
    for m in info['must_not_pin_dol']:
        print(f"  * {m['name']} = {m['addr']} ({m['source']})")
    print(f"d_basesNP Must-Not-Pin ({len(info['must_not_pin_bases'])}):")
    for m in info['must_not_pin_bases']:
        print(f"  * {m['name']} = {m['sec']}:{m['offset']} ({m['source']})")
    print(f"Other unlanded d_basesNP refs ({len(info['other_bases_refs'])}):")
    for o in info['other_bases_refs']:
        print(f"  ? {o['name']} = {o['sec']}:{o['offset']}")
