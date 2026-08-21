import struct
import os

dol = open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol', 'rb').read()
print('DOL sections (build_dol.py layout):')
for i in range(18):
    fo = struct.unpack_from('>I', dol, i * 4)[0]
    ad = struct.unpack_from('>I', dol, 0x48 + i * 4)[0]
    sz = struct.unpack_from('>I', dol, 0x90 + i * 4)[0]
    if sz:
        print('  %2d: fileoff 0x%08X addr 0x%08X size 0x%X' % (i, fo, ad, sz))
