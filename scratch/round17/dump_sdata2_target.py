import struct
import os

dol = open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol', 'rb').read()
S2_FO = 0x0034DA80
S2_AD = 0x80427980

# pool at 0x8042C130
off = S2_FO + (0x8042C130 - S2_AD)
raw = dol[off:off + 0x40]
print('target .sdata2 @0x8042C130:')
for j in range(0, 0x40, 4):
    w = struct.unpack_from('>I', raw, j)[0]
    f = struct.unpack_from('>f', raw, j)[0]
    print('  +0x%02X: 0x%08X = %g' % (j, w, f))
