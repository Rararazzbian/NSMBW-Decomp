import struct
import os

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
with open(DOL, 'rb') as fh:
    data = fh.read()

# (file offset, base addr, size) from slices/wiimj2d.json meta + header walk
SECTIONS = [
    ('.init',    0x00000100, 0x80004000, 0x26BC),
    ('.text',    0x000027C0, 0x80006780, 0x2E7544),
    ('.extab',   0x002E9D20, 0x800066C0, 0x48),
    ('.extabindex', 0x002E9D80, 0x80006720, 0x5C),
    ('.ctors',   0x002E9DE0, 0x802EDCE0, 0x2D0),
    ('.dtors',   0x002EA0C0, 0x802EDFC0, 0x18),
    ('.rodata',  0x002EA0E0, 0x802EDFE0, 0x106B0),
    ('.data',    0x002FA7A0, 0x802FE6A0, 0x532D0),
    ('.sdata',   0x0034DA80, 0x80427980, 0x251C),
    ('.sdata2',  0x0034FFA0, 0x8042B360, 0x4B44),
]

def fileoff(addr):
    for name, fo, base, size in SECTIONS:
        if base <= addr < base + size:
            return fo + (addr - base)
    return None

def u32(addr):
    fo = fileoff(addr)
    if fo is None:
        return None
    return struct.unpack_from('>I', data, fo)[0]

# sanity: .ctors entry at 0x802EDD94 -> 0x8007EC20 (__sinit_\d_bg_actor_mng_cpp)
print('.ctors[0x802EDD94] -> 0x%08X' % u32(0x802EDD94))

# sanity: vtable at 0x80310058 -> first slot = __dt__17dBgActorManager_cFv @ 0x8007E1D0
print('.data vtable[0x80310058] -> 0x%08X' % u32(0x80310058))
print('.data vtable[0x8031005C] -> 0x%08X' % u32(0x8031005C))
print('.data vtable[0x80310060] -> 0x%08X' % u32(0x80310060))

def dump(addr, size, label):
    fo = fileoff(addr)
    raw = data[fo:fo + size]
    with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17\%s.bin' % label, 'wb') as fh:
        fh.write(raw)
    print('%s: %d bytes at fileoff 0x%X' % (label, len(raw), fo))
    return raw

dump(0x802EFC68, 0x30, 'rodata_802EFC68')
dump(0x8030F820, 0x248, 'data_8030F820')
dump(0x8030FFF4, 0x70, 'data_8030FFF4')
dump(0x8042C130, 0x50, 'sdata2_8042C130')

print()
print('sbss region 0x8042A0B8-0x8042A0C0 is BSS (not in DOL file) - zeroed at runtime')
