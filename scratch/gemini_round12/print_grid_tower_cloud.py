import json

with open('scratch/gemini_round12/landing_kits_data.json') as f:
    data = json.load(f)

for name in ['d_a_wm_grid.cpp', 'd_a_wm_tower.cpp', 'd_a_wm_smallcloud.cpp']:
    print(f"\n======================================")
    print(f"=== {name} ===")
    print(f"======================================")
    info = data[name]
    print(f"Removals ({len(info['removals'])}): {info['removals']}")
    print(f"Additions ({len(info['dol_unpinned_additions'])}):")
    for a in info['dol_unpinned_additions']:
        print(f"  + {a['name']} = {a['addr']}")
    print(f"DOL Must-Not-Pin ({len(info['must_not_pin_dol'])}):")
    for m in info['must_not_pin_dol']:
        print(f"  * {m['name']} = {m['addr']} ({m['source']})")
    print(f"d_basesNP Must-Not-Pin ({len(info['must_not_pin_bases'])}):")
    for m in info['must_not_pin_bases']:
        print(f"  * {m['name']} = {m['sec']}:{m['offset']} ({m['source']})")
    print(f"DOL Pinned ({len(info['dol_pinned'])}):")
    for p in info['dol_pinned']:
        print(f"  P {p['name']} = {p['addr']}")
