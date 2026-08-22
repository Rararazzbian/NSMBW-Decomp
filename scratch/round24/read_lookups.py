import struct

with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol', 'rb') as f:
    data = f.read()

def read_u32_be(offset):
    return struct.unpack('>I', data[offset:offset+4])[0]

# Find which section contains 0x802EFBC0
for i in range(18):
    if i < 7:
        addr_off = 0x48 + i*4
        size_off = 0x90 + i*4
        file_off = 0x00 + i*4
    else:
        addr_off = 0x64 + (i-7)*4
        size_off = 0xAC + (i-7)*4
        file_off = 0x1C + (i-7)*4
    
    addr = read_u32_be(addr_off)
    size = read_u32_be(size_off)
    foff = read_u32_be(file_off)
    
    if addr <= 0x802EFBC0 < addr + size:
        print('Section %d: file_offset=0x%08X, addr=0x%08X, size=0x%08X' % (i, foff, addr, size))
        
        for lbl_name, lbl_addr in [('lbl_802EFBC0', 0x802EFBC0), ('lbl_802EFBD0', 0x802EFBD0), ('lbl_802EFBE0', 0x802EFBE0), ('lbl_802EFBF0', 0x802EFBF0)]:
            off = foff + (lbl_addr - addr)
            vals = struct.unpack('>4I', data[off:off+16])
            print('  %s @ 0x%08X: [%s]' % (lbl_name, lbl_addr, ', '.join('0x%08X' % v for v in vals)))
        break