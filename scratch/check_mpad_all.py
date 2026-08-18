import json, re

with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)
meta_sections = slice_data['meta']['sections']
sec_bases = {sname: int(sinfo['addr'], 16) for sname, sinfo in meta_sections.items()}
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
                'size': size,
                'comment': comment
            })

print("=== Checking .ctors around m_pad ===")
# Let's check all ctors
ctors = [s for s in symbols if s['sec'] == '.ctors']
for c in ctors:
    print(f"  .ctors: {hex(c['addr'])} {c['name']}")

print("\n=== Checking .dtors around m_pad ===")
dtors = [s for s in symbols if s['sec'] == '.dtors']
for d in dtors:
    print(f"  .dtors: {hex(d['addr'])} {d['name']}")

print("\n=== Checking text after 0x8016F860 ===")
for s in symbols:
    if s['sec'] == '.text' and 0x8016F800 <= s['addr'] <= 0x80170000:
        print(f"  .text: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

print("\n=== Checking data / rodata / sdata / sdata2 around m_pad ===")
# Does m_pad have any .rodata, .data, .sdata, .sdata2?
for sec in ['.rodata', '.data', '.sdata', '.sdata2']:
    syms = [s for s in symbols if s['sec'] == sec and ('mPad' in s['name'] or 'm_pad' in s['name'])]
    print(f"{sec} m_pad syms: {len(syms)}")
    for s in syms:
        print(f"  {s['sec']}: {hex(s['addr'])} {s['name']}")
