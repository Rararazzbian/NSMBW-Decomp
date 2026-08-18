import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / 'tools'))
from elffile import ElfFile, ElfRelaSec, ElfSymtab

obj_path = ROOT / 'bin' / 'dtkspl' / 'obj' / 'auto_03_800272F0_text.o'
elf = ElfFile.read(obj_path.read_bytes())

symtab = elf.get_section('.symtab')

for sec in elf.sections:
    if isinstance(sec, ElfRelaSec):
        print(f"\nRelocations section {sec.name} for {sec.header.sh_info}:")
        for rel in sec.relas:
            sym = symtab.syms[rel.r_sym]
            print(f"  offs=0x{rel.r_offset:04X} (0x{0x800272F0 + rel.r_offset:08X}), type={rel.r_type}, sym={sym.name}, addend=0x{rel.r_addend:X}")
