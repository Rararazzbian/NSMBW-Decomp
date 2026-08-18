import os
import re
import json

# 1. Load slices/wiimj2d.json
with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)

meta_sections = slice_data['meta']['sections']
sec_bases = {sname: int(sinfo['addr'], 16) for sname, sinfo in meta_sections.items()}
slices = slice_data['slices']

# Build landed ranges per section
landed_ranges = {sec: [] for sec in sec_bases}
for sl in slices:
    src = sl.get('source')
    non_matching = sl.get('nonMatching', False)
    if src and not non_matching:
        mr = sl.get('memoryRanges', {})
        for sec, r in mr.items():
            if sec in sec_bases:
                s_offs, e_offs = [int(x, 16) for x in r.split('-')]
                base = sec_bases[sec]
                landed_ranges[sec].append((base + s_offs, base + e_offs, src))

# 2. Load syms.txt
syms_txt = {}
with open('syms.txt') as f:
    for line_no, line in enumerate(f, 1):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if '=' in line:
            sym, addr_str = line.split('=', 1)
            addr_clean = addr_str.split(';')[0].split('#')[0].split('//')[0].strip()
            sym = sym.strip()
            syms_txt[sym] = (int(addr_clean, 16), line_no, line)

# 3. Load wiimj2d_symbols.txt
all_symbols = {}
symbols_by_addr = {}
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
            s_obj = {
                'name': name.strip(),
                'sec': sec.strip(),
                'addr': addr_int,
                'size': size,
                'comment': comment
            }
            all_symbols[name.strip()] = s_obj
            symbols_by_addr[addr_int] = s_obj

def is_addr_landed(sec, addr):
    for s_addr, e_addr, src in landed_ranges.get(sec, []):
        if s_addr <= addr < e_addr:
            return True, src
    return False, None

def analyze_tu_strict(tu_name, disasm_files, tu_ranges):
    print(f"\n=======================================================")
    print(f"=== LANDING KIT ANALYSIS: {tu_name} ===")
    print(f"=======================================================")
    
    # 1. syms.txt Removals
    print(f"\n--- 1. syms.txt REMOVALS for {tu_name} ---")
    removals = []
    for sym_name, (sym_addr, line_no, raw_line) in syms_txt.items():
        for sec, (s_addr, e_addr) in tu_ranges.items():
            if s_addr <= sym_addr < e_addr:
                removals.append((sym_name, sym_addr, sec, line_no, raw_line))
    
    print(f"Total removals found: {len(removals)}")
    for sym_name, sym_addr, sec, line_no, raw_line in sorted(removals, key=lambda x: x[1]):
        print(f"  {sym_name} = {hex(sym_addr)} (line {line_no}, {sec})")

    # 2. Extract referenced symbols from disassembly text files ONLY within tu_ranges['.text']
    text_start, text_end = tu_ranges['.text']
    referenced_syms = set()
    defined_syms = set()

    for fpath in disasm_files:
        with open(fpath, 'r', encoding='utf-8', errors='ignore') as f:
            current_addr = 0
            in_range = False
            for line in f:
                line = line.strip()
                # Check address comments: /* 8016F330 ... */
                m_addr = re.match(r'^/\*\s*([0-9a-fA-F]{8})\s+', line)
                if m_addr:
                    current_addr = int(m_addr.group(1), 16)
                    in_range = (text_start <= current_addr < text_end)
                
                # Check function definition
                m_fn = re.match(r'^\.fn\s+([^,]+)', line)
                if m_fn:
                    fn_name = m_fn.group(1).strip()
                    if in_range:
                        defined_syms.add(fn_name)

                if not in_range:
                    continue

                if '/*' in line and '*/' in line:
                    instr_part = line.split('*/', 1)[1].strip()
                    # match bl/b <sym>
                    m_bl = re.match(r'^(?:bl|b)\s+([a-zA-Z0-9_<>@$]+)', instr_part)
                    if m_bl:
                        sym = m_bl.group(1).strip()
                        if not sym.startswith('.L_') and not sym.startswith('lbl_'):
                            referenced_syms.add(sym)
                    
                    # match <sym>@(ha|l|sda21|sdarx|toc|sectoff)
                    for m_rel in re.finditer(r'["\']?([a-zA-Z0-9_<>@$]+)["\']?@(ha|l|sda21|sdarx|toc|sectoff)', instr_part):
                        sym = m_rel.group(1).strip()
                        referenced_syms.add(sym)

    print(f"\n--- 2. SYMBOL REFERENCE CLASSIFICATION ---")
    print(f"Defined in TU text: {len(defined_syms)}")
    print(f"Referenced in TU text: {len(referenced_syms)}")

    external_syms = set()
    for sym in referenced_syms:
        if sym in defined_syms:
            continue
        # Check if internal by address
        if sym in all_symbols:
            s_obj = all_symbols[sym]
            is_internal = False
            for sec, (s_addr, e_addr) in tu_ranges.items():
                if s_obj['sec'] == sec and s_addr <= s_obj['addr'] < e_addr:
                    is_internal = True
                    break
            if is_internal:
                continue
        # Also check synthetic names e.g. @14502_8042E010
        if '_' in sym and sym.rsplit('_', 1)[-1].startswith('80'):
            try:
                addr = int(sym.rsplit('_', 1)[-1], 16)
                is_internal = False
                for sec, (s_addr, e_addr) in tu_ranges.items():
                    if s_addr <= addr < e_addr:
                        is_internal = True
                        break
                if is_internal:
                    continue
            except:
                pass
        external_syms.add(sym)

    print(f"External referenced symbols: {len(external_syms)}")

    must_not_pin = []
    already_pinned = []
    must_add_pin = []
    unknown_syms = []

    for sym in sorted(external_syms):
        addr = None
        sec = None
        if sym in all_symbols:
            s_obj = all_symbols[sym]
            addr = s_obj['addr']
            sec = s_obj['sec']
        elif sym in syms_txt:
            addr, line_no, _ = syms_txt[sym]
            sec = '.text'
            if addr in symbols_by_addr:
                sec = symbols_by_addr[addr]['sec']
        elif '_' in sym and sym.rsplit('_', 1)[-1].startswith('80'):
            try:
                addr = int(sym.rsplit('_', 1)[-1], 16)
                if addr in symbols_by_addr:
                    sec = symbols_by_addr[addr]['sec']
                    sym = symbols_by_addr[addr]['name']
            except:
                pass

        if addr is not None and sec is not None:
            landed, land_src = is_addr_landed(sec, addr)
            if landed:
                must_not_pin.append((sym, addr, sec, land_src))
            else:
                if sym in syms_txt:
                    already_pinned.append((sym, addr, sec, syms_txt[sym][1]))
                else:
                    must_add_pin.append((sym, addr, sec))
        else:
            unknown_syms.append(sym)

    print(f"\n--- 3. MUST-NOT-PIN LIST (Defined in Landed Slices) [{len(must_not_pin)}] ---")
    for sym, addr, sec, land_src in sorted(must_not_pin, key=lambda x: x[1]):
        print(f"  {sym} = {hex(addr)} ({sec}, defined by {land_src})")

    print(f"\n--- 4. ALREADY PINNED IN syms.txt [{len(already_pinned)}] ---")
    for sym, addr, sec, line_no in sorted(already_pinned, key=lambda x: x[1]):
        print(f"  {sym} = {hex(addr)} ({sec}, line {line_no})")

    print(f"\n--- 5. syms.txt ADDITIONS (Must Add Pins) [{len(must_add_pin)}] ---")
    for sym, addr, sec in sorted(must_add_pin, key=lambda x: x[1]):
        print(f"  {sym} = {hex(addr)}; // {sec}")

    if unknown_syms:
        print(f"\n--- UNKNOWN / UNRESOLVED [{len(unknown_syms)}] ---")
        for sym in unknown_syms:
            print(f"  {sym}")

# Execute for m_pad
pad_disasms = [
    'scratch/disasm/auto_03_8016F330_text.o.txt',
    'scratch/disasm/auto_sinit__m_pad_cpp_text.o.txt',
    'scratch/disasm/auto_03_8016F808_text.o.txt'
]
pad_ranges = {
    '.text': (0x8016F330, 0x8016F880),
    '.ctors': (0x802EDEFC, 0x802EDF00),
    '.bss': (0x80377F88, 0x803780C8),
    '.sbss': (0x8042A740, 0x8042A760)
}
analyze_tu_strict("m_pad.cpp", pad_disasms, pad_ranges)

# Execute for coin_main
coin_disasms = [
    'scratch/disasm/auto_03_800272F0_text.o.txt',
    'scratch/disasm/auto_sinit__d_a_en_coin_m_text.o.txt'
]
coin_ranges = {
    '.text': (0x800272F0, 0x800281C0),
    '.ctors': (0x802EDD18, 0x802EDD1C),
    '.rodata': (0x802EE750, 0x802EE7F0),
    '.data': (0x80303078, 0x80303368),
    '.bss': (0x803530E8, 0x80353120),
    '.sdata2': (0x8042B630, 0x8042B638)
}
analyze_tu_strict("d_a_en_coin_main.cpp", coin_disasms, coin_ranges)

