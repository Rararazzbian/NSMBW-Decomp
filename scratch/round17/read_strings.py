import struct
import os

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
DATA_FILEOFF = 0x002FA7A0
DATA_BASE = 0x802FE6A0

with open(DOL, 'rb') as fh:
    data = fh.read()

for addr, n in [(0x8030FFF4, 0x30), (0x80310000, 0x30), (0x80310010, 0x30),
                (0x80310020, 0x30), (0x80310038, 0x30)]:
    fo = DATA_FILEOFF + (addr - DATA_BASE)
    raw = data[fo:fo + n]
    s = raw.split(b'\x00')[0]
    print('0x%X: %r' % (addr, s))
