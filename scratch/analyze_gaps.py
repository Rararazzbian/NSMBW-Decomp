import os
import re
import sys

sys.path.insert(0, os.path.abspath('.'))

# Load symbols from bin/dtk/wiimj2d_symbols.txt
sym_pattern = re.compile(r'^([^\s=]+)\s*=\s*([^:]+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$')
symbols = []
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8') as f:
    for line in f:
        m = sym_pattern.match(line.strip())
        if m:
            name, sec, addr_str, comment = m.groups()
            addr = int(addr_str, 16)
            size_m = re.search(r'size:(0x[0-9A-Fa-f]+|\d+)', comment)
            size = int(size_m.group(1), 16 if size_m.group(1).startswith('0x') else 10) if size_m else 0
            symbols.append({
                'name': name,
                'sec': sec,
                'addr': addr,
                'size': size,
                'comment': comment
            })

# Also parse dtk_splits_wiimj2d.txt
with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r', encoding='utf-8') as f:
    splits_text = f.read()

# Function to analyze a text range [start, end)
def analyze_range(start, end, label=""):
    fns = [s for s in symbols if s['sec'] == '.text' and start <= s['addr'] < end]
    sinits = [f for f in fns if '__sinit' in f['name']]
    classes = {}
    for f in fns:
        # extract class name from mangled
        m = re.search(r'__(\d+)([A-Za-z0-9_]+)', f['name'])
        if m:
            cname = m.group(2)[:int(m.group(1))]
            classes[cname] = classes.get(cname, 0) + 1
        elif '::' in f['name']:
            cname = f['name'].split('::')[0]
            classes[cname] = classes.get(cname, 0) + 1

    total_bytes = end - start
    fn_count = len(fns)
    b_per_fn = (total_bytes / fn_count) if fn_count > 0 else 0

    # Look for vtables in .data for the classes
    vtables = []
    for c in classes:
        vt_sym = f"__vt__{len(c)}{c}"
        vt_hits = [s for s in symbols if vt_sym in s['name']]
        if vt_hits:
            vtables.append((c, vt_hits[0]))

    return {
        'start': start,
        'end': end,
        'size': total_bytes,
        'fn_count': fn_count,
        'b_per_fn': b_per_fn,
        'sinits': len(sinits),
        'sinit_names': [s['name'] for s in sinits],
        'classes': classes,
        'vtables': vtables,
        'sample_fns': [f['name'] for f in fns[:8]]
    }

# Analyze all gaps between 500 bytes and 15000 bytes
gaps_to_check = [
    (0x80014330, 0x80014F10), # 3040 B
    (0x8001CBB0, 0x8001F6C0), # 11024 B
    (0x800272F0, 0x8002AB40), # 14416 B
    (0x8002EF50, 0x800311E0), # 8848 B
    (0x800331E0, 0x800356D0), # 9456 B
    (0x80036930, 0x80037EA0), # 5488 B
    (0x800451F0, 0x800460D0), # 3808 B
    (0x8005B3A0, 0x8005DFD0), # 11312 B
    (0x8005E9A0, 0x800613B0), # 10768 B (contains d_a_player_manager!)
    (0x800660C0, 0x80066FB0), # 3824 B
    (0x80069020, 0x8006C420), # 13312 B
    (0x800CA150, 0x800CD800), # 14000 B
    (0x800CED00, 0x800CFCE0), # 4064 B
    (0x800DF950, 0x800E1AA0), # 8528 B
    (0x800E2070, 0x800E46E0), # 9840 B
    (0x800E75D0, 0x800E8240), # 3184 B
    (0x800F2C00, 0x800F3570), # 2416 B
    (0x8010D270, 0x8010F080), # 7696 B
    (0x80124EB0, 0x80126650), # 6048 B
    (0x8016B090, 0x8016DBE0), # 11088 B
    (0x8016F330, 0x80170AC0), # 6032 B
    (0x801A9E30, 0x801ABFA0), # 8560 B
]

for g_start, g_end in gaps_to_check:
    res = analyze_range(g_start, g_end)
    print(f"=== Gap 0x{g_start:08X} - 0x{g_end:08X} ({res['size']} B, {res['fn_count']} fns, {res['b_per_fn']:.1f} B/fn, {res['sinits']} sinits) ===")
    print(f"  Classes: {res['classes']}")
    print(f"  Vtables: {[v[0] for v in res['vtables']]}")
    print(f"  Sinits: {res['sinit_names']}")
    print(f"  Sample fns: {res['sample_fns']}")
    print()
