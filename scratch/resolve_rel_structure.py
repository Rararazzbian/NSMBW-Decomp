import sys
from pathlib import Path

sys.path.append('tools')
from relfile import Rel

rel = Rel.read(Path('original/d_basesNP.rel').read_bytes())
print("Parsed REL successfully!")
print(f"Number of sections: {len(rel.sections)}")
for i, s in enumerate(rel.sections):
    print(f"  Sec {i}: exec={s.executable}, is_bss={s.is_bss}, len={hex(len(s.data))}")

print(f"Number of import blocks: {len(rel.relocations)}")
for mod_id, reloc_list in rel.relocations.items():
    print(f"  Module {mod_id}: {len(reloc_list)} relocations")

# Let's inspect relocations in section 2 (.ctors)
# In section 2 (.ctors), what are the relocations?
ctors_relocs = []
for mod_id, reloc_list in rel.relocations.items():
    curr_sec = 0
    curr_off = 0
    for r in reloc_list:
        curr_off += r.offset
        if r.reloc_type.name == 'R_RVL_SECT':
            curr_sec = r.section
            curr_off = 0
            continue
        if curr_sec == 2: # .ctors
            ctors_relocs.append((curr_off, mod_id, r.section, r.addend, r.reloc_type.name))

print(f"\nTotal relocations in .ctors (section 2): {len(ctors_relocs)}")
for off, mod_id, sec, addend, rtype in ctors_relocs:
    print(f"  .ctors +0x{off:03x} -> mod={mod_id} sec={sec} addend=0x{addend:06x} ({rtype})")

