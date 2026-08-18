import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent / 'tools'))
from elffile import ElfFile, ElfSymtab, STB, STT

ROOT = Path(__file__).resolve().parent.parent.parent
v_o = ROOT / 'wip' / 'player_manager' / '_v_assembled.o'

elf = ElfFile.read(v_o.read_bytes())
symtab = elf.get_section('.symtab')

# Let's collect all symbols located in .text
text_idx = None
for idx, sec in enumerate(elf.sections):
    if sec.name == '.text':
        text_idx = idx
        break

text_syms = []
for sym in symtab.syms:
    if sym.st_shndx == text_idx:
        text_syms.append(sym)

text_syms.sort(key=lambda s: s.st_value)

print("Symbols in .text of _v_assembled.o:")
for s in text_syms:
    bind = 'GLOBAL' if s.st_info_bind == STB.STB_GLOBAL else ('WEAK' if s.st_info_bind == STB.STB_WEAK else 'LOCAL')
    print(f"  0x{s.st_value:04X} - 0x{s.st_value + s.st_size:04X} (size 0x{s.st_size:03X}) [{bind:6s}] {s.name}")
