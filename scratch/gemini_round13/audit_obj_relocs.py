import os
import sys
import subprocess
import re

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from elffile import ElfFile

dtk = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
grid_obj = os.path.join(ROOT, 'scratch', 'gemini_round13', 'd_a_wm_grid.o')
tower_obj = os.path.join(ROOT, 'scratch', 'gemini_round13', 'd_a_wm_tower.o')

# Load alias_db
alias_db = {}
with open(os.path.join(ROOT, 'alias_db.txt')) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'): continue
        if '=' in line:
            k, v = line.split('=', 1)
            alias_db[k.strip()] = v.strip()

# Load wiimj2d_symbols
dol_syms = {}
with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#') or line.startswith('//'): continue
        m = re.match(r'^(\S+)\s*=\s*(\.\w+):0x([0-9a-fA-F]+);', line)
        if m:
            dol_syms[m.group(1)] = (m.group(2), int(m.group(3), 16))

# Load d_basesNP_symbols
bases_syms = {}
with open(os.path.join(ROOT, 'bin', 'dtk', 'd_basesNP_symbols.txt')) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#') or line.startswith('//'): continue
        m = re.match(r'^(\S+)\s*=\s*(\.\w+):0x([0-9a-fA-F]+);', line)
        if m:
            bases_syms[m.group(1)] = (m.group(2), int(m.group(3), 16))

def inspect_obj(name, path):
    print(f"\n======================================")
    print(f"=== {name} ({path}) ===")
    print(f"======================================")
    elf = ElfFile(open(path, 'rb').read())
    
    # Check all symbols in elf
    print("\nSymbols in object:")
    for sym in elf.symtab.symbols:
        if sym.name:
            print(f"  {sym.name:50s} sec:{sym.shndx:2d} bind:{sym.binding:8s} type:{sym.type:8s}")
            
    # Check all relocations
    print("\nRelocations:")
    for sec in elf.sections:
        if sec.name.startswith('.rela'):
            for rel in sec.relocations:
                sym_name = elf.symtab.symbols[rel.sym_idx].name if rel.sym_idx < len(elf.symtab.symbols) else f"sym_{rel.sym_idx}"
                # check where it resolves
                res_type = "UNKNOWN"
                if sym_name in dol_syms:
                    res_type = f"DOL ({dol_syms[sym_name][0]} 0x{dol_syms[sym_name][1]:08x})"
                elif sym_name in bases_syms:
                    res_type = f"REL local ({bases_syms[sym_name][0]} 0x{bases_syms[sym_name][1]:06x})"
                elif sym_name in alias_db.values():
                    res_type = "alias_db matched"
                print(f"  {sec.name:15s} offset:0x{rel.offset:04x} type:{rel.type:2d} sym:{sym_name} -> {res_type}")

inspect_obj("d_a_wm_grid.o", grid_obj)
inspect_obj("d_a_wm_tower.o", tower_obj)
