import json
import re
import sys

# Load slices
with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)

meta_sections = slice_data['meta']['sections']
sec_bases = {}
for sname, sinfo in meta_sections.items():
    sec_bases[sname] = int(sinfo['addr'], 16)

print("Section Virtual Base Addresses:")
for sname, base in sec_bases.items():
    print(f"  {sname}: {hex(base)}")

# Parse symbols
symbols_by_sec = {}
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
            addr_int = int(addr, 16)
            if sec not in symbols_by_sec:
                symbols_by_sec[sec] = []
            symbols_by_sec[sec].append({
                'name': name.strip(),
                'addr': addr_int,
                'size': size,
                'comment': comment
            })

for sec in symbols_by_sec:
    symbols_by_sec[sec].sort(key=lambda s: s['addr'])

print("\n--- Investigating m_pad.cpp ---")
# m_pad text: 0x8016F330 .. 0x8016F860
# Let's check symbols around text 0x8016F330
pad_text_syms = [s for s in symbols_by_sec.get('.text', []) if 0x8016F000 <= s['addr'] <= 0x80170000]
for s in pad_text_syms:
    print(f"  .text: {hex(s['addr'])}..{hex(s['addr']+s['size'])} (size {hex(s['size'])}) {s['name']}")

# Let's check all sections for m_pad
for sec in ['.ctors', '.rodata', '.data', '.bss', '.sdata', '.sbss', '.sdata2']:
    print(f"\nSymbols in {sec} around m_pad ranges:")
    if sec == '.bss':
        syms = [s for s in symbols_by_sec.get(sec, []) if 0x80377E00 <= s['addr'] <= 0x80378200]
    elif sec == '.sbss':
        syms = [s for s in symbols_by_sec.get(sec, []) if 0x8042A600 <= s['addr'] <= 0x8042A800]
    elif sec == '.data':
        syms = [s for s in symbols_by_sec.get(sec, []) if 0x80329000 <= s['addr'] <= 0x8032B000]
    elif sec == '.ctors':
        syms = [s for s in symbols_by_sec.get(sec, []) if 0x802EDC00 <= s['addr'] <= 0x802EDF00]
    elif sec == '.rodata':
        syms = [s for s in symbols_by_sec.get(sec, []) if 0x802EE000 <= s['addr'] <= 0x802F5000]
    elif sec == '.sdata':
        syms = [s for s in symbols_by_sec.get(sec, []) if 0x80427800 <= s['addr'] <= 0x80429000]
    elif sec == '.sdata2':
        syms = [s for s in symbols_by_sec.get(sec, []) if 0x8042B000 <= s['addr'] <= 0x8042D000]
    else:
        syms = []
    for s in syms:
        print(f"  {sec}: {hex(s['addr'])}..{hex(s['addr']+s['size'])} (size {hex(s['size'])}) {s['name']}")
