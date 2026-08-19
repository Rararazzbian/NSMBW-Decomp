import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
sys.path.insert(0, 'tools')
from relfile import Rel
from elfconsts import PPC_RELOC_TYPE

REL_PATH = 'original/d_basesNP.rel'
with open(REL_PATH, 'rb') as f:
    rel = Rel(0, file=f)

TEXT_SEC = 1
CTORS_SEC = 2
FUNC_LO = 0x16c150
FUNC_HI = 0x16d290

SEC_NAMES = {1:'.text',2:'.ctors',3:'.dtors',4:'.rodata',5:'.data',6:'.bss'}

targets = {}
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
        if curr_section == TEXT_SEC and FUNC_LO <= curr_pos < FUNC_HI and module == rel.index:
            tsec = SEC_NAMES.get(reloc.section, str(reloc.section))
            targets.setdefault(tsec, set()).add(reloc.addend)

for sec in sorted(targets):
    vals = sorted(targets[sec])
    print(sec, 'min', hex(vals[0]), 'max', hex(vals[-1]), 'count', len(vals))
    for v in vals:
        print('   ', hex(v))
