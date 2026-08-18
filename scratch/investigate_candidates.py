import os
import re
import sys

sys.path.insert(0, os.path.abspath('.'))

# Parse all sections in dtk_splits_wiimj2d.txt
with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r', encoding='utf-8') as f:
    lines = f.readlines()

units = []
current_unit = None

for line in lines:
    line = line.rstrip()
    if not line or line.startswith('Sections:'):
        continue
    if not line.startswith('\t') and line.endswith(':'):
        unit_name = line[:-1]
        current_unit = {'name': unit_name, 'sections': {}}
        units.append(current_unit)
    elif line.startswith('\t') and current_unit is not None:
        parts = line.strip().split()
        sec_name = parts[0]
        sec_info = {}
        for p in parts[1:]:
            if ':' in p:
                k, v = p.split(':', 1)
                sec_info[k] = int(v, 16) if v.startswith('0x') else v
        current_unit['sections'][sec_name] = sec_info

def find_brackets(sec_name, start_addr, end_addr):
    # Find TU with end == start_addr (left bracket) and TU with start == end_addr (right bracket)
    left = None
    right = None
    for u in units:
        if sec_name in u['sections']:
            s = u['sections'][sec_name].get('start')
            e = u['sections'][sec_name].get('end')
            if e == start_addr:
                left = (u['name'], e)
            if s == end_addr:
                right = (u['name'], s)
    return left, right

# Also load symbols
sym_pattern = re.compile(r'^([^\s=]+)\s*=\s*([^:]+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$')
symbols = []
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8') as f:
    for line in f:
        m = sym_pattern.match(line.strip())
        if m:
            name, sec, addr_str, comment = m.groups()
            symbols.append({
                'name': name,
                'sec': sec,
                'addr': int(addr_str, 16),
                'comment': comment
            })

def investigate_candidate(text_start, text_end, proposed_name):
    print(f"==================================================")
    print(f"Candidate: {proposed_name}")
    print(f".text range: 0x{text_start:08X} - 0x{text_end:08X} (size: 0x{text_end - text_start:X} = {text_end - text_start} bytes)")
    
    # Left and right .text brackets
    left_t, right_t = find_brackets('.text', text_start, text_end)
    print(f"  .text left bracket:  {left_t}")
    print(f"  .text right bracket: {right_t}")

    # Find symbols in all sections that fall in this TU
    fns = [s for s in symbols if s['sec'] == '.text' and text_start <= s['addr'] < text_end]
    sinits = [f for f in fns if '__sinit' in f['name']]
    print(f"  Function count: {len(fns)}")
    print(f"  Bytes per function: {(text_end - text_start)/len(fns):.1f}")
    print(f"  __sinit count: {len(sinits)} ({[s['name'] for s in sinits]})")

    # Find classes and vtables
    classes = set()
    for f in fns:
        m = re.search(r'__(\d+)([A-Za-z0-9_]+)', f['name'])
        if m:
            classes.add(m.group(2)[:int(m.group(1))])
    print(f"  Classes referenced: {classes}")
    
    vtables = []
    for c in classes:
        vt_sym = f"__vt__{len(c)}{c}"
        hits = [s for s in symbols if vt_sym in s['name']]
        if hits:
            vtables.append((c, hits[0]))
    print(f"  Vtables in map: {[v[0] + ' @ 0x' + hex(v[1]['addr']) for v in vtables]}")

    # Check non-text symbols
    # Look for data / rodata / bss / sdata / sbss / sdata2 associated with classes or __sinit
    print("  Non-text symbols for classes/sinit:")
    for s in symbols:
        if s['sec'] != '.text':
            for c in classes:
                if c in s['name']:
                    print(f"    {s['sec']} 0x{s['addr']:08X} {s['name']}")

    print()

candidates = [
    (0x8010D270, 0x8010F080, "d_WarningManager.cpp"),
    (0x800F2C00, 0x800F3570, "d_wm_connect.cpp"),
    (0x800E75D0, 0x800E8240, "d_tencoin_mng.cpp"),
    (0x8016F330, 0x80170AC0, "m_pad.cpp"),
    (0x80124EB0, 0x80126650, "d_a_mask.cpp"),
    (0x800CED00, 0x800CFCE0, "d_nand_thread.cpp"),
]

for start, end, name in candidates:
    investigate_candidate(start, end, name)
