import struct, re, json

with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)
sec_bases = {sname: int(sinfo['addr'], 16) for sname, sinfo in slice_data['meta']['sections'].items()}
slices = slice_data['slices']

symbols = []
with open('bin/dtk/wiimj2d_symbols.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        m = re.match(r'^([^=]+)=\s*([^:]+):(0x[0-9a-fA-F]+);\s*//\s*(.*)$', line)
        if m:
            name, sec, addr, comment = m.groups()
            size_m = re.search(r'size:(0x[0-9a-fA-F]+)', comment)
            size = int(size_m.group(1), 16) if size_m else 0
            symbols.append({
                'name': name.strip(),
                'sec': sec.strip(),
                'addr': int(addr, 16),
                'size': size
            })

print("=== Symbols in .sdata around 0x80427B50 ===")
for s in symbols:
    if s['sec'] == '.sdata' and 0x80427B00 <= s['addr'] <= 0x80427C00:
        print(f"  {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

print("\n=== Slices with .sdata around 0x80427B50 ===")
base = sec_bases['.sdata']
target_offs = 0x80427B50 - base
print(f".sdata base = {hex(base)}, target offs = {hex(target_offs)}")
for sl in slices:
    mr = sl.get('memoryRanges', {})
    if '.sdata' in mr:
        r = mr['.sdata']
        s_val, e_val = [int(x, 16) for x in r.split('-')]
        if (target_offs - 0x100) <= s_val <= (target_offs + 0x100) or (target_offs - 0x100) <= e_val <= (target_offs + 0x100):
            print(f"  {sl.get('source', sl.get('sliceName'))}: .sdata={r} (0x{s_val:x}-0x{e_val:x})")

