import os
import sys
import json
import re
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from elffile import ElfFile

def load_symbols(path):
    syms = []
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
                syms.append({'name': name, 'sec': sec, 'addr': addr, 'size': size})
    return syms

def main():
    boss_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'd_en_bossNP_symbols.txt'))
    text_syms = [s for s in boss_syms if s['sec'] == '.text']
    text_syms.sort(key=lambda s: s['addr'])
    
    split_dir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_en_bossNP', 'obj')
    
    # Read all ELF files in split_dir
    files = sorted(os.listdir(split_dir))
    
    print("=== Inspecting DTK Split Objects for d_en_bossNP ===")
    for f in files:
        if not f.endswith('.o') or f.startswith('auto_03_') or f.startswith('auto_04_') or f.startswith('auto_05_'):
            continue
        p = os.path.join(split_dir, f)
        try:
            elf = ElfFile.read(open(p, 'rb').read())
            # Find symbols in elf
            sec_text = elf.get_section('.text')
            sec_size = sec_text.size() if sec_text else 0
            symtab = elf.get_section('.symtab')
            sym_names = []
            if symtab:
                sym_names = [s.name for s in symtab.symbols if s.name and not s.name.startswith('.')]
            print(f"{f:<35} text_size=0x{sec_size:04x} ({sec_size:5d} B), symbols={len(sym_names)}: {sym_names[:3]}")
        except Exception as e:
            print(f"{f:<35} ERROR: {e}")

if __name__ == '__main__':
    main()
