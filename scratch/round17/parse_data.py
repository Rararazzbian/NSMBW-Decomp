import struct

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
with open(DOL, 'rb') as fh:
    data = fh.read()

DATA_FILEOFF = 0x002FA7A0
DATA_BASE = 0x802FE6A0


def fileoff(addr):
    return DATA_FILEOFF + (addr - DATA_BASE)


def dump(addr, size, label):
    fo = fileoff(addr)
    raw = data[fo:fo + size]
    with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17\%s.bin' % label, 'wb') as fh:
        fh.write(raw)
    print('%s: %d bytes at fileoff 0x%X' % (label, len(raw), fo))


# full unit .data: l_object_name(0x40) l_Pa3_rail(0x380) l_Pa3_MG(0x260)
# l_Pa3_daishizen(0x1A0) l_rail_list(0x14) strings(0x64) vtable(0xC) = 0x244
dump(0x8030F820, 0x244, 'data_full')

# parse l_object_name + rail entries: 0x20-byte elements
for base, name, count in ((0x8030F820, 'l_object_name', 2),
                          (0x8030F860, 'l_Pa3_rail', 0x1C),
                          (0x8030FBE0, 'l_Pa3_MG_house_ami_rail', 0x13),
                          (0x8030FE40, 'l_Pa3_daishizen', 0x0D)):
    print('\n=== %s (%d entries) ===' % (name, count))
    for i in range(count):
        a = base + i * 0x20
        fo = fileoff(a)
        unit = struct.unpack_from('>I', data, fo)[0]
        name16 = struct.unpack_from('>H', data, fo + 4)[0]
        flag = struct.unpack_from('>H', data, fo + 6)[0]
        offx = struct.unpack_from('>f', data, fo + 8)[0]
        offy = struct.unpack_from('>f', data, fo + 12)[0]
        offz = struct.unpack_from('>f', data, fo + 16)[0]
        sizx = struct.unpack_from('>f', data, fo + 20)[0]
        sizy = struct.unpack_from('>f', data, fo + 24)[0]
        param = struct.unpack_from('>I', data, fo + 28)[0]
        print('  [%02d] unit=0x%04X name=0x%04X flag=0x%04X off=(%g,%g,%g) size=(%g,%g) param=0x%08X' %
              (i, unit, name16, flag, offx, offy, offz, sizx, sizy, param))
