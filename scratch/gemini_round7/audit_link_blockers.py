import os, sys, re, json

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

# 1. Load disassemblies
with open('scratch/gemini_round7/auto_03_8016F330_text.o.txt') as f:
    d1 = f.read()
with open('scratch/gemini_round7/auto_03_8016F808_text.o.txt') as f:
    d2 = f.read()

disasm_text = d1 + '\n' + d2

# Find all called symbols
called_syms = set()
for line in disasm_text.splitlines():
    m = re.search(r'\bbl\s+([^\s,]+)', line)
    if m:
        called_syms.add(m.group(1))

# Also find sda21 external references like sInstance__Q23EGG17CoreControllerMgr
for line in disasm_text.splitlines():
    m = re.search(r'([A-Za-z0-9_@\$<>]+)@sda21', line)
    if m:
        called_syms.add(m.group(1))

# Also direct memory references
for line in disasm_text.splitlines():
    m = re.search(r'lis\s+r\d+,\s*([A-Za-z0-9_@\$<>]+)@ha', line)
    if m:
        called_syms.add(m.group(1))

# 2. Parse wiimj2d_symbols.txt
sym_file = os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')
pat = re.compile(r'^(\S+)\s*=\s*(\.[^:]+):(0x[0-9A-Fa-f]+);\s*(.*)$')
size_pat = re.compile(r'size:(0x[0-9A-Fa-f]+|\d+)')

symbol_map = {}
with open(sym_file, 'r', encoding='utf-8', errors='replace') as f:
    for line in f:
        m = pat.match(line.strip())
        if m:
            name, sec, addr_s, rest = m.groups()
            addr = int(addr_s, 16)
            symbol_map[name] = (sec, addr)

# 3. Parse existing syms.txt
existing_pins = {}
with open(os.path.join(ROOT, 'syms.txt'), 'r', encoding='utf-8') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if '=' in line:
            name, _, addr_s = line.partition('=')
            name = name.strip()
            addr = int(addr_s.strip(), 16)
            existing_pins[name] = addr

# 4. Parse banked slices from slices/wiimj2d.json
with open(os.path.join(ROOT, 'slices', 'wiimj2d.json'), 'r') as f:
    slices_data = json.load(f)

meta_sec = slices_data['meta']['sections']
sec_bases = {}
for sname, sinfo in meta_sec.items():
    addr = int(sinfo['addr'], 16)
    off = int(sinfo.get('offset', '0'), 16)
    sec_bases[sname] = addr + off

banked_intervals = []
for s in slices_data['slices']:
    src = s['source']
    mr = s.get('memoryRanges', {})
    for sec_name, rng in mr.items():
        base = sec_bases.get(sec_name, 0)
        a_str, b_str = rng.split('-')
        start_addr = base + int(a_str, 16)
        end_addr = base + int(b_str, 16)
        banked_intervals.append((start_addr, end_addr, src, sec_name))

def is_banked(addr):
    for start, end, src, sec in banked_intervals:
        if start <= addr < end:
            return True, src, sec
    return False, None, None

print(f"Total symbols checked: {len(called_syms)}")
print("\n=== CLASSIFICATION OF SYMBOLS ===")

internal_syms = []
banked_syms = []
already_pinned = []
need_pin = []
unknown_syms = []

for sym in sorted(called_syms):
    # Clean quotes
    clean_sym = sym.strip('"')
    if clean_sym in symbol_map:
        sec, addr = symbol_map[clean_sym]
        # Check if in m_pad.cpp ranges
        if (sec == '.text' and 0x8016F330 <= addr < 0x80170AC0) or \
           (sec == '.data' and 0x80329F60 <= addr < 0x80329F70) or \
           (sec == '.bss' and 0x80377F88 <= addr < 0x803780C8) or \
           (sec == '.sbss' and 0x8042A740 <= addr < 0x8042A760) or \
           (sec == '.sdata2' and 0x8042E010 <= addr < 0x8042E030):
            internal_syms.append((clean_sym, sec, addr))
            continue
        
        banked, b_src, b_sec = is_banked(addr)
        if banked:
            banked_syms.append((clean_sym, hex(addr), b_src, b_sec))
        elif clean_sym in existing_pins:
            already_pinned.append((clean_sym, hex(addr)))
        else:
            need_pin.append((clean_sym, hex(addr), sec))
    else:
        # Check if helper like _savegpr or __destroy_arr
        if clean_sym in existing_pins:
            already_pinned.append((clean_sym, hex(existing_pins[clean_sym])))
        else:
            unknown_syms.append(clean_sym)

print(f"1. Internal to m_pad.cpp: {len(internal_syms)}")
for s, sec, a in internal_syms:
    print(f"   - {s} ({sec}:{hex(a)})")

print(f"\n2. Banked in existing slices (DO NOT PIN): {len(banked_syms)}")
for s, a, src, sec in banked_syms:
    print(f"   - {s} ({a} in {src} [{sec}])")

print(f"\n3. Already in syms.txt: {len(already_pinned)}")
for s, a in already_pinned:
    print(f"   - {s} ({a})")

print(f"\n4. Proposed Pins to ADD to syms.txt: {len(need_pin)}")
for s, a, sec in need_pin:
    print(f"   - {s} = {a}")

print(f"\n5. Unresolved / Special: {len(unknown_syms)}")
for s in unknown_syms:
    print(f"   - {s}")

# Check symbols defined by m_pad.cpp that might be currently in syms.txt
print("\n=== SYMBOLS TO REMOVE FROM syms.txt UPON LANDING ===")
removals = []
for name, (sec, addr) in symbol_map.items():
    if (sec == '.text' and 0x8016F330 <= addr < 0x80170AC0) or \
       (sec == '.data' and 0x80329F60 <= addr < 0x80329F70) or \
       (sec == '.bss' and 0x80377F88 <= addr < 0x803780C8) or \
       (sec == '.sbss' and 0x8042A740 <= addr < 0x8042A760):
        if name in existing_pins:
            removals.append((name, sec, hex(addr)))

print(f"Found {len(removals)} symbols to remove:")
for name, sec, addr_s in removals:
    print(f"   - {name} ({sec}:{addr_s})")
