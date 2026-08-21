import struct

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
with open(DOL, 'rb') as fh:
    data = fh.read(0x100)

for i in range(0, 0x100, 16):
    hexs = ' '.join('%02X' % b for b in data[i:i+16])
    print('%04X  %s' % (i, hexs))
