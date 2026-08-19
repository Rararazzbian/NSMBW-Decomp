import sys, os
sys.path.insert(0, 'tools')
from relfile import Rel
from elfconsts import PPC_RELOC_TYPE

with open('original/d_basesNP.rel', 'rb') as f:
    rel = Rel(0, file=f)

RODATA_SEC = 4
LO, HI = 0x8b10, 0x8bb0

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
        if curr_section == RODATA_SEC and LO <= curr_pos < HI:
            print(f'.rodata+{hex(curr_pos)} -> module={module} type={reloc.reloc_type.name} target_section={reloc.section} addend={hex(reloc.addend)}')
