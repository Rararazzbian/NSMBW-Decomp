import os
import sys
import json
import re
import subprocess
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from relfile import Rel, RelSection, RelRelocation
from elffile import ElfFile

def load_symbols(path):
    syms = []
    # e.g. fn_2_110 = .text:0x00000110; // type:function size:0x30
    sym_re = re.compile(r'^(\S+)\s*=\s*(\.\w+):0x([0-9a-fA-F]+);\s*(?://\s*(.*))?$')
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('//'):
                continue
            m = sym_re.match(line)
            if m:
                name = m.group(1)
                sec = m.group(2)
                addr = int(m.group(3), 16)
                meta = m.group(4) or ''
                size = 0
                sz_m = re.search(r'size:0x([0-9a-fA-F]+)', meta)
                if sz_m:
                    size = int(sz_m.group(1), 16)
                stype = 'unknown'
                st_m = re.search(r'type:(\w+)', meta)
                if st_m:
                    stype = st_m.group(1)
                scope = 'local'
                sc_m = re.search(r'scope:(\w+)', meta)
                if sc_m:
                    scope = sc_m.group(1)
                syms.append({
                    'name': name,
                    'sec': sec,
                    'addr': addr,
                    'size': size,
                    'type': stype,
                    'scope': scope,
                    'raw': line
                })
    return syms

def main():
    print("Parsing symbols and slices...")
    # Load d_basesNP symbols
    bases_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt'))
    print(f"d_basesNP symbols: {len(bases_syms)}")
    
    # Load d_en_bossNP symbols
    boss_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'd_en_bossNP_symbols.txt'))
    print(f"d_en_bossNP symbols: {len(boss_syms)}")
    
    # Load alias_db
    alias_db = {}
    with open(os.path.join(ROOT, 'alias_db.txt'), 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if '=' in line:
                k, v = line.split('=', 1)
                alias_db[k.strip()] = v.strip()
                
    # Load wiimj2d symbols
    dol_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'))
    dol_by_name = {s['name']: s for s in dol_syms}
    dol_by_addr = {(s['sec'], s['addr']): s for s in dol_syms}
    
    # Let's inspect the 4 units of d_basesNP:
    # 1. d_a_wm_grid.cpp
    # 2. d_a_wm_tower.cpp
    # 3. d_a_wm_smallcloud.cpp
    # 4. d_a_wm_kinoko_base.cpp
    
    print("\n--- Task A Units Inspection ---")
    targets = {
        'd_a_wm_grid.cpp': {
            '.text': (0x164230, 0x164430),
            '.ctors': (0x3e4, 0x3e8),
            '.rodata': (0x88b8, 0x88d0),
            '.data': (0x44cb4, 0x44d54),
            '.bss': (0xfdd0, 0xfde0),
        },
        'd_a_wm_tower.cpp': {
            '.text': (0x185710, 0x185b70),
            '.ctors': (0x44c, 0x450),
            '.rodata': (0x9488, 0x94a0),
            '.data': (0x480b4, 0x4818c),
            '.bss': (0x10a98, 0x10aa8),
        },
        'd_a_wm_smallcloud.cpp': {
            '.text': (0x1797e0, 0x179ff0),
            '.ctors': (0x430, 0x434),
            '.rodata': (0x8f58, 0x8fa0),
            '.data': (0x4728c, 0x47484),
            '.bss': (0x10130, 0x10140),
        },
        'd_a_wm_kinoko_base.cpp': {
            '.text': (0x16b2d0, 0x16bda0),
            '.ctors': (0x3fc, 0x400),
            '.rodata': (0x8ac8, 0x8af0), # wait, let's verify exact rodata bounds (in r10 it was 8b70, in r11 8ac8)
            '.data': (0x458c0, 0x45a90),
            '.bss': (0xfe80, 0xfe90),
        }
    }
    
    # Check symbols in each range
    for tu_name, ranges in targets.items():
        print(f"\nTarget: {tu_name}")
        for sec, (start, end) in ranges.items():
            syms_in_range = [s for s in bases_syms if s['sec'] == sec and start <= s['addr'] < end]
            print(f"  {sec}: 0x{start:x}-0x{end:x} (span 0x{end-start:x}, {len(syms_in_range)} symbols)")
            for s in syms_in_range:
                alias = alias_db.get(f"R_2_{sec_to_idx(sec)}_{s['addr']:x}", "")
                alias_str = f" [alias: {alias}]" if alias else ""
                print(f"    0x{s['addr']:06x} {s['size']:04x} {s['name']}{alias_str}")

def sec_to_idx(sec):
    mapping = {'': 0, '.text': 1, '.ctors': 2, '.dtors': 3, '.rodata': 4, '.data': 5, '.bss': 6}
    return mapping.get(sec, 0)

if __name__ == '__main__':
    main()
