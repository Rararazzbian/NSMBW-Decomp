import re
import json
from collections import defaultdict

# 1. Load symbols in d_basesNP_symbols.txt
symbols_by_sec = defaultdict(list)
with open('bin/dtk/d_basesNP_symbols.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        m = re.match(r'^([^=]+)=\s*([^:]+):(0x[0-9a-fA-F]+);\s*//\s*(.*)$', line)
        if m:
            name, sec, addr, comment = m.groups()
            size_m = re.search(r'size:(0x[0-9a-fA-F]+)', comment)
            size = int(size_m.group(1), 16) if size_m else 0
            symbols_by_sec[sec].append({
                'name': name.strip(),
                'sec': sec.strip(),
                'addr': int(addr, 16),
                'size': size,
                'comment': comment
            })

for sec in symbols_by_sec:
    symbols_by_sec[sec].sort(key=lambda s: s['addr'])

# 2. Load alias_db.txt
alias_map = {}
with open('alias_db.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if '=' in line:
            k, v = line.split('=', 1)
            alias_map[k.strip()] = v.strip()

# 3. Load slices/d_basesNP.json
with open('slices/d_basesNP.json') as f:
    slice_data = json.load(f)

landed_slices = slice_data['slices']

print("=== Landed Slices in d_basesNP.json ===")
for s in landed_slices:
    if s.get('source'):
        print(f"  {s.get('source')}: {s.get('memoryRanges')}")

# 4. Search for all distinct actor/class groups in d_basesNP
# In .data (sec_idx 5), find all profiles: R_2_5_<off> -> g_profile_<NAME>
profiles = []
for k, v in alias_map.items():
    if k.startswith('R_2_5_') and v.startswith('g_profile_'):
        off = int(k.split('_')[3], 16)
        pname = v[len('g_profile_'):]
        profiles.append((off, pname, v))

profiles.sort()
print(f"\nTotal profiles in d_basesNP: {len(profiles)}")
for off, pname, full_name in profiles[:30]:
    print(f"  .data: 0x{off:05x} -> {full_name}")

