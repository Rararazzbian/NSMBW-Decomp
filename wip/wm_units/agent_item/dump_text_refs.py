import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..', 'tools'))
from relfile import Rel
from elfconsts import PPC_RELOC_TYPE

REL_PATH = os.path.join(os.path.dirname(__file__), '..', '..', '..', 'original', 'd_basesNP.rel')
with open(REL_PATH, 'rb') as f:
    rel = Rel(0, file=f)

TEXT_SEC = 1
FUNC_LO = 0x167120
FUNC_HI = 0x16793C

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
        if curr_section == TEXT_SEC and FUNC_LO <= curr_pos < FUNC_HI:
            print(f'.text+{hex(curr_pos)} -> module={module} type={reloc.reloc_type.name} target_section={reloc.section} addend={hex(reloc.addend)}')
