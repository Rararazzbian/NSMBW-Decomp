import sys, os
sys.path.insert(0, os.path.join('tools'))
from relfile import Rel
from elfconsts import PPC_RELOC_TYPE

REL_PATH = os.path.join('original', 'd_basesNP.rel')
with open(REL_PATH, 'rb') as f:
    rel = Rel(0, file=f)

TEXT_SEC = 1
CTORS_SEC = None
TEXT_LO = 0x167940
TEXT_HI = 0x1686e0

# find section index of .ctors by scanning section table if available
for i, sec in enumerate(rel.sections):
    pass

targets_by_section = {}
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
        if curr_section == TEXT_SEC and TEXT_LO <= curr_pos < TEXT_HI:
            targets_by_section.setdefault(reloc.section, set()).add(reloc.addend)

for sec, addrs in sorted(targets_by_section.items()):
    print('section', sec, 'min', hex(min(addrs)), 'max', hex(max(addrs)), 'count', len(addrs))
