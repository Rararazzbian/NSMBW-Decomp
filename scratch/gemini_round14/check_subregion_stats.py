import json
import re

symbols = []
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('//') or '=' not in line:
            continue
        parts = line.split('=', 1)
        sym_name = parts[0].strip()
        rest = parts[1].strip()
        m = re.match(r'(\.[a-zA-Z0-9_\$]+):0x([0-9a-fA-F]+);\s*(//\s*size:0x([0-9a-fA-F]+))?', rest)
        if m:
            sec = m.group(1)
            addr = int(m.group(2), 16)
            size = int(m.group(4), 16) if m.group(4) else 0
            symbols.append({'name': sym_name, 'sec': sec, 'addr': addr, 'size': size, 'raw': line})

sec_symbols = {}
for s in symbols:
    sec_symbols.setdefault(s['sec'], []).append(s)
for sec in sec_symbols:
    sec_symbols[sec].sort(key=lambda x: (x['addr'], x['size']))

text_syms = [s for s in sec_symbols.get('.text', []) if 0x8006CF40 <= s['addr'] < 0x8008C200]
for i in range(len(text_syms)):
    curr = text_syms[i]
    if i + 1 < len(text_syms):
        curr['size'] = text_syms[i+1]['addr'] - curr['addr']
    else:
        curr['size'] = 0x8008C200 - curr['addr']

# Define Candidate TU text boundaries to analyze:
# Option A (5 TUs):
# 1. d_bc.cpp: 0x8006CF40 - 0x80076BC0 (or 0x80076FD0)
# 2. d_bg.cpp: 0x80076FD0 (or 0x80076BC0) - 0x8007E180
# 3. d_bg_actor_mng.cpp: 0x8007E180 - 0x8007F7A0
# 4. d_bg_unit.cpp: 0x8007F7A0 - 0x800872E0 (or 0x80089150)
# 5. d_capture_mng.cpp: 0x800872E0 (or 0x80089150) - 0x8008C200

# Let us check the symbol named fractions for each sub-region!
subregions = [
    ("d_bc.cpp", 0x8006CF40, 0x80076BC0),
    ("d_beans_kuribo_mng", 0x80076BC0, 0x80076FD0),
    ("d_bg.cpp", 0x80076FD0, 0x8007E180),
    ("d_bg_actor_mng.cpp", 0x8007E180, 0x8007F7A0),
    ("d_bg_unit.cpp (main)", 0x8007F7A0, 0x800872E0),
    ("d_bg_tex_mng / d_block_mng / d_boat_log", 0x800872E0, 0x80088FD0),
    ("d_capture_mng.cpp (m3d/dDOF/dCam/dCapture...)", 0x80088FD0, 0x8008C200)
]

for name, start, end in subregions:
    syms = [s for s in text_syms if start <= s['addr'] < end]
    named = [s for s in syms if not (s['name'].startswith('fn_') or s['name'].startswith('lbl_'))]
    anon = [s for s in syms if s['name'].startswith('fn_') or s['name'].startswith('lbl_')]
    total_bytes = sum(s['size'] for s in syms)
    named_bytes = sum(s['size'] for s in named)
    anon_bytes = sum(s['size'] for s in anon)
    pct_syms = (len(named) / len(syms) * 100) if syms else 0
    pct_bytes = (named_bytes / total_bytes * 100) if total_bytes else 0
    print(f"=== {name} (0x{start:08x} - 0x{end:08x}, size 0x{end-start:05x} = {end-start:,} bytes) ===")
    print(f"  Symbols: {len(syms)} total ({len(named)} named, {len(anon)} anon) -> {pct_syms:.1f}% named")
    print(f"  Bytes:   {total_bytes:,} total ({named_bytes:,} named, {anon_bytes:,} anon) -> {pct_bytes:.1f}% named")
    print(f"  First sym: {syms[0]['name'] if syms else 'NONE'}")
    print(f"  Last sym:  {syms[-1]['name'] if syms else 'NONE'}")

