import sys
sys.path.insert(0, 'tools')
from relfile import Rel
from elfconsts import PPC_RELOC_TYPE

with open('original/d_basesNP.rel', 'rb') as f:
    rel = Rel(0, file=f)

BSS_SEC = 6
TARGET = 0x11b70

for module, relocs in rel.relocations.items():
    curr_pos = 0
    curr_section = 0
    for reloc in relocs:
        if reloc.reloc_type == PPC_RELOC_TYPE.R_RVL_SECT:
            curr_pos = 0
            curr_section = reloc.section
            continue
        if reloc.reloc_type == PPC_RELOC_TYPE.R_RVL_STOP:
            continue
        curr_pos += reloc.offset
        if reloc.section == BSS_SEC and reloc.addend == TARGET:
            print(f'referrer: module={module} src_section={curr_section} src_offset={hex(curr_pos)} -> .bss {hex(TARGET)}')
