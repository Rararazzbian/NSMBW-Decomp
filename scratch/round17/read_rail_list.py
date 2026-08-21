import struct

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
with open(DOL, 'rb') as fh:
    data = fh.read()

DATA_FILEOFF = 0x002FA7A0
DATA_BASE = 0x802FE6A0


def u32(addr):
    fo = DATA_FILEOFF + (addr - DATA_BASE)
    return struct.unpack_from('>I', data, fo)[0]


# l_rail_list @ 0x8030FFE0 (0x14 = 5 pointers)
for i in range(5):
    v = u32(0x8030FFE0 + 4 * i)
    print('l_rail_list[%d] -> 0x%08X' % (i, v))

# vtable at 0x80310058
print('vtable[0]: 0x%08X' % u32(0x80310058))
print('vtable[1]: 0x%08X' % u32(0x8031005C))
print('vtable[2]: 0x%08X' % u32(0x80310060))

# l_object_name first entry layout check: unit/name/flag as u32/u16/u16
fo = DATA_FILEOFF + (0x8030F820 - DATA_BASE)
print('l_object_name[0] raw:', data[fo:fo+0x20].hex(' '))
