import os
import sys
import json
import re
from pathlib import Path

sys.path.append('tools')
from elffile import ElfFile, SHT, STB, STT
from slicelib import load_slice_file

# 1. Load slices/wiimj2d.json
with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)

meta_sections = slice_data['meta']['sections']
sec_bases = {sname: int(sinfo['addr'], 16) for sname, sinfo in meta_sections.items()}
slices = slice_data['slices']

# Build landed address ranges for each section
landed_ranges = {sec: [] for sec in sec_bases}
for sl in slices:
    # A slice is landed if source is present and not nonMatching
    src = sl.get('source')
    non_matching = sl.get('nonMatching', False)
    if src and not non_matching:
        mr = sl.get('memoryRanges', {})
        for sec, r in mr.items():
            if sec in sec_bases:
                s_offs, e_offs = [int(x, 16) for x in r.split('-')]
                base = sec_bases[sec]
                landed_ranges[sec].append((base + s_offs, base + e_offs, src))

print(f"Loaded {len(slices)} slices. Landed .text ranges: {len(landed_ranges['.text'])}")

# 2. Load syms.txt
syms_txt = {}
with open('syms.txt') as f:
    for line_no, line in enumerate(f, 1):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if '=' in line:
            sym, addr_str = line.split('=', 1)
            # Remove comments from addr_str if any
            addr_clean = addr_str.split(';')[0].split('#')[0].split('//')[0].strip()
            sym = sym.strip()
            syms_txt[sym] = (int(addr_clean, 16), line_no, line)

print(f"Loaded {len(syms_txt)} symbols from syms.txt")

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

def analyze_tu(tu_name, obj_paths, tu_ranges):
    print(f"\n=======================================================")
    print(f"=== LANDING KIT ANALYSIS: {tu_name} ===")
    print(f"=======================================================")
    
    # 1. Check syms.txt removals
    print(f"\n--- 1. syms.txt REMOVALS for {tu_name} ---")
    removals = []
    for sym_name, (sym_addr, line_no, raw_line) in syms_txt.items():
        for sec, (s_addr, e_addr) in tu_ranges.items():
            if s_addr <= sym_addr < e_addr:
                removals.append((sym_name, sym_addr, sec, line_no, raw_line))
    
    print(f"Total removals found: {len(removals)}")
    for sym_name, sym_addr, sec, line_no, raw_line in sorted(removals, key=lambda x: x[1]):
        print(f"  {sym_name}={hex(sym_addr)} (line {line_no}, {sec})")

    # 2. Extract relocations from objects
    print(f"\n--- 2. EXTRACTING RELOCATIONS FROM DTKSPL OBJECTS ---")
    referenced_syms = set()
    tu_defined_syms = set()
    
    for obj_p in obj_paths:
        elf_bytes = Path(obj_p).read_bytes()
        elf = ElfFile.read(elf_bytes)
        symtab = elf.get_section('.symtab')
        if not symtab:
            continue
        
        # Collect symbols defined in this obj
        for sym in symtab.syms:
            if sym.st_shndx not in [0, 0xFFF1, 0xFFF2]: # not UNDEF/ABS/COMMON
                tu_defined_syms.add(sym.name)
        
        # Collect relocations
        for sec in elf.sections:
            if sec.name.startswith('.rela.') or sec.name.startswith('.rel.'):
                for rel in sec.relocs:
                    sym = symtab.syms[rel.sym_idx]
                    referenced_syms.add(sym.name)
    
    print(f"TU defined symbols in split objects: {len(tu_defined_syms)}")
    print(f"TU referenced symbols (raw relocations): {len(referenced_syms)}")
    
    # External symbols = referenced - defined in TU
    external_syms = set()
    for sym in referenced_syms:
        if sym in tu_defined_syms:
            continue
        # Also check if it's within TU range by address
        if sym in all_symbols:
            s_obj = all_symbols[sym]
            is_internal = False
            for sec, (s_addr, e_addr) in tu_ranges.items():
                if s_obj['sec'] == sec and s_addr <= s_obj['addr'] < e_addr:
                    is_internal = True
                    break
            if is_internal:
                continue
        external_syms.add(sym)
    
    print(f"External referenced symbols: {len(external_syms)}")
    
    # Classify external symbols
    must_not_pin = []
    already_pinned = []
    must_add_pin = []
    unknown_syms = []
    
    for sym in sorted(external_syms):
        if sym in all_symbols:
            s_obj = all_symbols[sym]
            addr = s_obj['addr']
            sec = s_obj['sec']
            landed, land_src = is_addr_landed(sec, addr)
            if landed:
                must_not_pin.append((sym, addr, sec, land_src))
            else:
                if sym in syms_txt:
                    already_pinned.append((sym, addr, sec, syms_txt[sym][1]))
                else:
                    must_add_pin.append((sym, addr, sec))
        elif sym in syms_txt:
            addr, line_no, _ = syms_txt[sym]
            # check if landed
            # (assume .text if unknown)
            landed, land_src = is_addr_landed('.text', addr)
            if landed:
                must_not_pin.append((sym, addr, '.text', land_src))
            else:
                already_pinned.append((sym, addr, '.text', line_no))
        else:
            unknown_syms.append(sym)
            
    print(f"\n--- 3. MUST-NOT-PIN LIST (Defined in Landed Slices) [{len(must_not_pin)}] ---")
    for sym, addr, sec, land_src in sorted(must_not_pin, key=lambda x: x[1]):
        print(f"  {sym} = {hex(addr)} ({sec}, defined by {land_src})")
        
    print(f"\n--- 4. ALREADY PINNED IN syms.txt [{len(already_pinned)}] ---")
    for sym, addr, sec, line_no in sorted(already_pinned, key=lambda x: x[1]):
        print(f"  {sym} = {hex(addr)} ({sec}, syms.txt line {line_no})")
        
    print(f"\n--- 5. syms.txt ADDITIONS (Must Add Pins) [{len(must_add_pin)}] ---")
    for sym, addr, sec in sorted(must_add_pin, key=lambda x: x[1]):
        print(f"  {sym} = {hex(addr)}; // {sec}")
        
    if unknown_syms:
        print(f"\n--- UNKNOWN SYMBOLS [{len(unknown_syms)}] ---")
        for sym in unknown_syms:
            print(f"  {sym}")

# Run for m_pad.cpp
pad_ranges = {
    '.text': (0x8016F330, 0x8016F880),
    '.ctors': (0x802EDEFC, 0x802EDF00),
    '.bss': (0x80377F88, 0x803780C8),
    '.sbss': (0x8042A740, 0x8042A760)
}
pad_objs = [
    'bin/dtkspl/obj/auto_03_8016F330_text.o',
    'bin/dtkspl/obj/auto_sinit__m_pad_cpp_text.o',
    'bin/dtkspl/obj/auto_03_8016F808_text.o'
]
analyze_tu("m_pad.cpp", pad_objs, pad_ranges)

# Run for d_a_en_coin_main.cpp
coin_ranges = {
    '.text': (0x800272F0, 0x800281C0),
    '.ctors': (0x802EDD18, 0x802EDD1C),
    '.rodata': (0x802EE750, 0x802EE7F0), # let's verify if 0x802EE7F0 or 0x802EE810
    '.data': (0x80303078, 0x80303368),
    '.bss': (0x803530E8, 0x80353120),
    '.sdata2': (0x8042B630, 0x8042B638)
}
coin_objs = [
    'bin/dtkspl/obj/auto_03_800272F0_text.o',
    'bin/dtkspl/obj/auto_sinit__d_a_en_coin_m_text.o'
]
analyze_tu("d_a_en_coin_main.cpp", coin_objs, coin_ranges)

