import struct
import os

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
if not os.path.exists(DOL):
    # try common alternates
    for cand in [r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol',
                 r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\wiimj2d.dol']:
        if os.path.exists(cand):
            DOL = cand
            break

with open(DOL, 'rb') as fh:
    data = fh.read()

def u32(off):
    return struct.unpack_from('>I', data, off)[0]

text_off = [u32(0 + 4 * i) for i in range(7)]
text_addr = [u32(0x1C + 4 * i) for i in range(7)]
text_size = [u32(0x38 + 4 * i) for i in range(7)]
data_off = [u32(0x54 + 4 * i) for i in range(11)]
data_addr = [u32(0x80 + 4 * i) for i in range(11)]
data_size = [u32(0x9C + 4 * i) for i in range(11)]
bss_addr = u32(0xB8)
bss_size = u32(0xBC)

names = ['init', 'text', 'extab', 'extabindex', 'ctors', 'dtors', 'rodata']
print('text-like sections:')
for i in range(7):
    print('  %-10s file 0x%08X addr 0x%08X size 0x%X' % (names[i], text_off[i], text_addr[i], text_size[i]))
names2 = ['data', 'sdata', 'sdata2', '?3', '?4', '?5', '?6', '?7', '?8', '?9', '?10']
print('data-like sections:')
for i in range(11):
    if data_size[i]:
        print('  %-10s file 0x%08X addr 0x%08X size 0x%X' % (names2[i], data_off[i], data_addr[i], data_size[i]))
print('bss addr 0x%08X size 0x%X' % (bss_addr, bss_size))

# map address -> (section, fileoff)
def addr_to_fileoff(addr):
    for i in range(7):
        if text_addr[i] and text_addr[i] <= addr < text_addr[i] + text_size[i]:
            return text_off[i] + (addr - text_addr[i])
    for i in range(11):
        if data_addr[i] and data_addr[i] <= addr < data_addr[i] + data_size[i]:
            return data_off[i] + (addr - data_addr[i])
    return None

# .ctors pointer at 0x802EDD94
fo = addr_to_fileoff(0x802EDD94)
print('\n.ctors[0x802EDD94] fileoff 0x%X -> value 0x%08X' % (fo, u32(fo)))

# dumps
def dump(addr, size, label):
    fo = addr_to_fileoff(addr)
    if fo is None:
        print('%s: no mapping' % label)
        return
    raw = data[fo:fo + size]
    with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17\%s.bin' % label, 'wb') as fh:
        fh.write(raw)
    print('%s: wrote %d bytes (fileoff 0x%X)' % (label, len(raw), fo))

dump(0x802EFC68, 0x30, 'rodata_802EFC68')
dump(0x8030F820, 0x248, 'data_8030F820')   # l_object_name..l_rail_list
dump(0x8030FFF4, 0x70, 'data_8030FFF4')    # 5 strings + vtable
dump(0x8042A0B8, 0x8, 'sbss_8042A0B8')
dump(0x8042C130, 0x50, 'sdata2_8042C130')
