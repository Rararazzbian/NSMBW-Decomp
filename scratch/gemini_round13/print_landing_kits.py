import json
import re
import os

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

dol_syms = {}
with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#') or line.startswith('//'): continue
        m = re.match(r'^(\S+)\s*=\s*(\.\w+):0x([0-9a-fA-F]+);', line)
        if m:
            dol_syms[int(m.group(3), 16)] = m.group(1)

with open(os.path.join(ROOT, 'scratch', 'gemini_round13', 'final_landing_kits.json')) as f:
    kits = json.load(f)

for tu, data in kits.items():
    print(f"\n==========================================")
    print(f"=== {tu} ===")
    print(f"==========================================")
    print(f"Slice Entry:")
    print(json.dumps(data['slice_entry'], indent=2))
    
    print(f"\nOwned symbols to be replaced/claimed ({len(data['owned_symbols'])}):")
    for s in data['owned_symbols']:
        print(f"  {s['sec']:8s} 0x{s['addr']:06x} size:0x{s['size']:04x} {s['name']}")

    print(f"\nDOL Must-Not-Pin ({len(data['must_not_pin_dol'])}):")
    for m in data['must_not_pin_dol']:
        print(f"  * {m['name']} = {m['addr']} ({m['source']})")

    print(f"\nDOL Unpinned Additions ({len(data['dol_unpinned_additions'])}):")
    for u in data['dol_unpinned_additions']:
        addr = int(u['addr'], 16)
        name = dol_syms.get(addr, u['name'])
        print(f"  + {name:60s} = 0x{addr:08X}")

    print(f"\nREL Must-Not-Pin ({len(data['must_not_pin_bases'])}):")
    for m in data['must_not_pin_bases']:
        print(f"  * {m['name']} = {m['sec']}:{m['offset']} ({m['source']})")
