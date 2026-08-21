import struct
import os

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
DATA_FILEOFF = 0x002FA7A0
DATA_BASE = 0x802FE6A0

with open(DOL, 'rb') as fh:
    data = fh.read()

def dump(addr, n):
    fo = DATA_FILEOFF + (addr - DATA_BASE)
    raw = data[fo:fo+n]
    words = [struct.unpack_from('>I', raw, i)[0] for i in range(0, len(raw), 4)]
    fl = [struct.unpack_from('>f', raw, i)[0] for i in range(0, len(raw), 4)]
    return raw.hex(), words, fl

for name, addr, n in [('l_object_name', 0x8030F820, 0x40),
                      ('l_Pa3_rail', 0x8030F860, 0x80),
                      ('l_rail_list', 0x8030FFE0, 0x14)]:
    hx, words, fl = dump(addr, n)
    print('=== %s @0x%X (%d bytes)' % (name, addr, n))
    for i, w in enumerate(words):
        print('  +0x%02X: 0x%08X  (%.6g)' % (i*4, w, fl[i]))
