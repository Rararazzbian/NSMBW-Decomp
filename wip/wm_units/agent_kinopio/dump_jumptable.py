import sys
sys.path.insert(0, 'tools')
from relfile import Rel
from elfconsts import PPC_RELOC_TYPE

with open('original/d_basesNP.rel', 'rb') as f:
    rel = Rel(0, file=f)

DATA_SEC = 5
LO, HI = 0x45d14, 0x45d68  # jumptable_2_data_45D14 through lbl_2_data_45D68 (secondary vtable)

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
        if curr_section == DATA_SEC and LO <= curr_pos < HI:
            print(f'.data+{hex(curr_pos)} -> module={module} type={reloc.reloc_type.name} target_section={reloc.section} addend={hex(reloc.addend)}')
