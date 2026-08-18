import struct

# Read wiimj2d.dol
with open('original/wiimj2d.dol', 'rb') as f:
    dol_data = f.read()

# Let's inspect .ctors section
# .ctors base = 0x802EDCE0, size = 0x2D0 + 0x4 = 0x2D4 (from wiimj2d.json: addr=0x802edce0, size=0x2d0, offset=0x4)
# In DOL: section 9 is .ctors
# Let's read the dol header to find offset of section 9
# DOL header format:
# 7 text section file offsets, 11 data section file offsets
# 7 text section mem addrs, 11 data section mem addrs
# 7 text section sizes, 11 data section sizes

offsets = struct.unpack('>18I', dol_data[0:72])
addrs = struct.unpack('>18I', dol_data[72:144])
sizes = struct.unpack('>18I', dol_data[144:216])

ctors_idx = -1
for i in range(18):
    if addrs[i] == 0x802EDCE0:
        ctors_idx = i
        break

print(f".ctors section index in DOL: {ctors_idx}")
if ctors_idx != -1:
    c_off = offsets[ctors_idx]
    c_addr = addrs[ctors_idx]
    c_size = sizes[ctors_idx]
    print(f".ctors file offset: {hex(c_off)}, mem addr: {hex(c_addr)}, size: {hex(c_size)}")
    
    # Read each 4-byte pointer in .ctors
    print("Listing all pointers in .ctors:")
    for slot_off in range(0, c_size, 4):
        ptr = struct.unpack('>I', dol_data[c_off + slot_off : c_off + slot_off + 4])[0]
        slot_addr = c_addr + slot_off
        slot_slice_offs = slot_off # relative to 0x802EDCE0
        print(f"  slot {hex(slot_addr)} (offs {hex(slot_slice_offs)}): -> {hex(ptr)}")

