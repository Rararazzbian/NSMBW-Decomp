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

print("=== Text before 0x8016F330 ===")
for s in symbols:
    if s['sec'] == '.text' and 0x8016EE00 <= s['addr'] <= 0x8016F340:
        print(f"  .text: {hex(s['addr'])} (size {hex(s['size'])}) {s['name']}")

print("\n=== Checking slices surrounding m_pad in slices/wiimj2d.json ===")
# m_pad text: 0x8016F330 .. 0x8016F880 -> offs = 0x8016F330 - 0x80006780 = 0x168BB0 .. 0x169100
pad_text_start_offs = 0x8016F330 - 0x80006780
pad_text_end_offs = 0x8016F880 - 0x80006780
print(f"pad .text offs: {hex(pad_text_start_offs)} - {hex(pad_text_end_offs)}")

# Let's find neighbors in wiimj2d.json
for s in slices:
    mr = s.get('memoryRanges', {})
    if '.text' in mr:
        r = mr['.text']
        s_val, e_val = [int(x, 16) for x in r.split('-')]
        if (pad_text_start_offs - 0x2000) <= s_val <= (pad_text_end_offs + 0x2000) or (pad_text_start_offs - 0x2000) <= e_val <= (pad_text_end_offs + 0x2000):
            print(f"  Slice {s.get('source', s.get('sliceName'))}: .text={r} (0x{s_val:x}-0x{e_val:x}) nonMatching={s.get('nonMatching')}")

