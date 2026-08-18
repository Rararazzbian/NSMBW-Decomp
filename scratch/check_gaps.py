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

print("=== Checking symbols in gaps around coin_main ===")
print("Data gap 0x80303018 .. 0x80303078:")
for s in symbols:
    if s['sec'] == '.data' and 0x80303000 <= s['addr'] <= 0x80303090:
        print(f"  {s['sec']}: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

print("Sdata2 gap 0x8042b638 .. 0x8042b690:")
for s in symbols:
    if s['sec'] == '.sdata2' and 0x8042b620 <= s['addr'] <= 0x8042b690:
        print(f"  {s['sec']}: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

print("\n=== Finding m_pad.cpp symbols and neighbors across all sections ===")
# Let's inspect all m_pad symbols:
m_pad_syms = [s for s in symbols if 'm_pad' in s['name'] or 'mPad' in s['name'] or (s['sec']=='.text' and 0x8016F330 <= s['addr'] <= 0x8016F860)]
print("m_pad text symbols:")
for s in m_pad_syms:
    print(f"  {s['sec']}: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

# Let's check ctors for m_pad:
print("\nCtors around 0x802EDE00:")
for s in symbols:
    if s['sec'] == '.ctors' and 0x802EDDF0 <= s['addr'] <= 0x802EDE60:
        print(f"  {s['sec']}: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

# Let's check bss around m_pad:
print("\nBSS around 0x80377F00:")
for s in symbols:
    if s['sec'] == '.bss' and 0x80377E00 <= s['addr'] <= 0x80378150:
        print(f"  {s['sec']}: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

# Let's check sbss around m_pad:
print("\nSBSS around 0x8042A700:")
for s in symbols:
    if s['sec'] == '.sbss' and 0x8042A700 <= s['addr'] <= 0x8042A790:
        print(f"  {s['sec']}: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

