import struct
import os

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
with open(DOL, 'rb') as fh:
    data = fh.read()

# From the sequential section layout (verified in scratch/round17/read_dol.py run):
#   init @0x100, text @0x27C0, extab @0x2E9D20, extabindex @0x2E9D80,
#   ctors @0x2E9DE0, dtors @0x2EA0C0, rodata @0x2EA0E0, data @0x2FA7A0,
#   sdata @0x34DA80, sdata2 @0x34FFA0
SECTIONS = {
    '.init':    (0x00000100, 0x80004000),
    '.text':    (0x000027C0, 0x80006780),
    '.extab':   (0x002E9D20, 0x800066C0),
    '.extabindex': (0x002E9D80, 0x80006720),
    '.ctors':   (0x002E9DE0, 0x802EDCE0),
    '.dtors':   (0x002EA0C0, 0x802EDFC0),
    '.rodata':  (0x002EA0E0, 0x802EDFE0),
    '.data':    (0x002FA7A0, 0x802FE6A0),
    '.sdata':   (0x0034DA80, 0x80427980),
    '.sdata2':  (0x0034FFA0, 0x8042B360),
}

def fileoff(addr):
    for sec, (fo, base) in SECTIONS.items():
        if base <= addr < base + 0x1000000 and (base <= addr):
            # determine size from the NEXT section's file offset
            return fo + (addr - base)
    return None

# sanity: read .ctors entry at 0x802EDD94
fo = fileoff(0x802EDD94)
v = struct.unpack_from('>I', data, fo)[0]
print('.ctors[0x802EDD94] -> 0x%08X (expect 0x8007EC20)' % v)

# sanity: vtable at 0x80310058
fo = fileoff(0x80310058)
v = struct.unpack_from('>I', data, fo)[0]
print('.data vtable[0x80310058] -> 0x%08X (expect 0x8007E1D0)' % v)

def dump(addr, size, label):
    fo = fileoff(addr)
    raw = data[fo:fo + size]
    with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17\%s.bin' % label, 'wb') as fh:
        fh.write(raw)
    print('%s: wrote %d bytes at fileoff 0x%X' % (label, len(raw), fo))

dump(0x802EFC68, 0x30, 'rodata_802EFC68')
dump(0x8030F820, 0x248, 'data_8030F820')
dump(0x8030FFF4, 0x70, 'data_8030FFF4')
dump(0x8042A0B8, 0x8, 'sbss_8042A0B8')
dump(0x8042C130, 0x50, 'sdata2_8042C130')
