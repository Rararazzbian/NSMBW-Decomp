import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent / 'tools'))
from elffile import ElfFile, ElfSymtab, STB, STT

ROOT = Path(__file__).resolve().parent.parent.parent
v_o = ROOT / 'wip' / 'player_manager' / '_v_assembled.o'

if v_o.exists():
    elf = ElfFile.read(v_o.read_bytes())
    text_sec = elf.get_section('.text')
    print(f".text size in _v_assembled.o: {hex(text_sec.header.sh_size)}")
    symtab = elf.get_section('.symtab')
    
    weak_funcs = []
    global_funcs = []
    for sym in symtab.syms:
        if sym.st_info_type == STT.STT_FUNC:
            if sym.st_info_bind == STB.STB_WEAK:
                weak_funcs.append((sym.name, sym.st_size, hex(sym.st_value)))
            elif sym.st_info_bind == STB.STB_GLOBAL:
                global_funcs.append((sym.name, sym.st_size, hex(sym.st_value)))
                
    print(f"\nGlobal functions count: {len(global_funcs)}")
    total_global_size = sum(sym[1] for sym in global_funcs)
    print(f"Total global function size: 0x{total_global_size:X} ({total_global_size} bytes)")
    
    print(f"\nWeak functions count: {len(weak_funcs)}")
    total_weak_size = sum(sym[1] for sym in weak_funcs)
    print(f"Total weak function size: 0x{total_weak_size:X} ({total_weak_size} bytes)")
    for name, size, val in weak_funcs:
        print(f"  - {name} (size 0x{size:X}, val {val})")
        
    print(f"\nTarget slice claim: 0x2A10")
    print(f"Compiled .text total: 0x{text_sec.header.sh_size:X}")
    print(f"Global total: 0x{total_global_size:X}")
    print(f"Global total vs Claim (0x2A10): diff = 0x{total_global_size - 0x2A10:X} ({total_global_size - 0x2A10} bytes)")
