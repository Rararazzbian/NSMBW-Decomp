import struct

with open('original/d_basesNP.rel', 'rb') as f:
    rel = f.read()

# Section 2 (.ctors) is at file offset 0x1c60f4, length 0x4fc
ctors_off = 0x1c60f4
ctors_len = 0x4fc

print(f"Total slots in .ctors: {ctors_len // 4}")

# Read relocation table for d_basesNP.rel to see what each slot in .ctors relocates to
# REL relocations:
# impOffset, impSize at header 0x28, 0x2C
imp_off, imp_size = struct.unpack('>II', rel_data[0x28:0x30])
print(f"impOffset: {hex(imp_off)}, impSize: {hex(imp_size)}")

# Read imports
# struct RelImport { u32 moduleId; u32 offset; }
num_imports = imp_size // 8
imports = []
for i in range(num_imports):
    mod_id, r_off = struct.unpack('>II', rel_data[imp_off + i*8 : imp_off + i*8 + 8])
    imports.append((mod_id, r_off))
    print(f"  Import {i}: moduleId={mod_id}, offset={hex(r_off)}")

# Parse internal relocations (moduleId == 2 or moduleId == 0)
# Let's write a relocation parser for REL
