import struct
import os

# Generate the four file-scope BgObjName_t array declarations for
# d_bg_actor_mng.cpp, combining:
#  - int fields (unit/name/flag/param) parsed from .data
#  - float fields (offset/size) parsed from __sinit__ (runtime-written)

DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
DATA_FILEOFF = 0x002FA7A0
DATA_BASE = 0x802FE6A0

with open(DOL, 'rb') as fh:
    data = fh.read()


def u32(addr):
    fo = DATA_FILEOFF + (addr - DATA_BASE)
    return struct.unpack_from('>I', data, fo)[0]


def u16(addr):
    fo = DATA_FILEOFF + (addr - DATA_BASE)
    return struct.unpack_from('>H', data, fo)[0]


# floats per element: global index -> (off, size) extracted from __sinit__
FLOATS = {
    0: ((8, -8, 0), (32, 32)), 1: ((0, 0, 0), (0, 0)),
    2: ((8, -8, 0), (32, 32)), 3: ((8, -8, 0), (32, 32)),
    4: ((8, -8, 0), (32, 32)), 5: ((8, -8, 0), (32, 32)),
    6: ((8, 0, 0), (24, 48)), 7: ((8, 0, 0), (32, 48)),
    8: ((8, -8, 0), (32, 32)), 9: ((8, -8, 0), (32, 32)),
    10: ((16, -8, 0), (48, 32)), 11: ((16, -8, 0), (48, 32)),
    12: ((8, -8, 0), (32, 32)), 13: ((16, 0, 0), (48, 48)),
    14: ((16, 0, 0), (48, 48)), 15: ((16, -16, 0), (48, 48)),
    16: ((0, -16, 0), (48, 48)), 17: ((8, -8, 0), (32, 32)),
    18: ((8, -8, 0), (32, 32)), 19: ((8, -8, 0), (32, 32)),
    20: ((8, -8, 0), (32, 32)), 21: ((8, -16, 0), (32, 48)),
    22: ((16, -8, 0), (48, 32)), 23: ((16, -8, 0), (48, 32)),
    24: ((8, -16, 0), (32, 48)), 25: ((8, -8, 0), (32, 32)),
    26: ((8, -8, 0), (32, 32)), 27: ((8, -8, 0), (32, 32)),
    28: ((8, -8, 0), (32, 32)), 29: ((0, 0, 0), (0, 0)),
    30: ((8, -8, 0), (32, 32)), 31: ((8, -8, 0), (32, 32)),
    32: ((8, -8, 0), (32, 32)), 33: ((8, -8, 0), (32, 32)),
    34: ((8, -8, 0), (32, 32)), 35: ((8, -8, 0), (32, 32)),
    36: ((8, -16, 0), (32, 48)), 37: ((16, -8, 0), (48, 32)),
    38: ((16, -8, 0), (48, 32)), 39: ((8, -16, 0), (32, 48)),
    40: ((16, 0, 0), (48, 48)), 41: ((16, 0, 0), (48, 48)),
    42: ((16, -16, 0), (48, 48)), 43: ((0, -16, 0), (48, 48)),
    44: ((8, -8, 0), (32, 32)), 45: ((8, -8, 0), (32, 32)),
    46: ((8, -8, 0), (32, 32)), 47: ((8, -8, 0), (32, 32)),
    48: ((0, 0, 0), (0, 0)),
    49: ((8, -8, 0), (32, 32)), 50: ((8, -8, 0), (32, 32)),
    51: ((8, -8, 0), (32, 32)), 52: ((8, -8, 0), (32, 32)),
    53: ((8, -16, 0), (32, 48)), 54: ((16, -8, 0), (48, 32)),
    55: ((16, -8, 0), (48, 32)), 56: ((8, -16, 0), (32, 48)),
    57: ((8, -8, 0), (32, 32)), 58: ((8, -8, 0), (32, 32)),
    59: ((8, -8, 0), (32, 32)), 60: ((8, -8, 0), (32, 32)),
    61: ((0, 0, 0), (0, 0)),
}

ARRAYS = [
    ('l_object_name', 0x8030F820, 2),
    ('l_Pa3_rail', 0x8030F860, 0x1C),
    ('l_Pa3_MG_house_ami_rail', 0x8030FBE0, 0x13),
    ('l_Pa3_daishizen', 0x8030FE40, 0x0D),
]


def fmt(f):
    if f == int(f):
        return '%d' % f
    return '%g' % f


out = []
global_idx = 0
for name, base, count in ARRAYS:
    out.append('static BgObjName_t %s[%d] = {' % (name, count))
    for i in range(count):
        addr = base + i * 0x20
        unit = u32(addr)
        nm = u16(addr + 4)
        flag = u16(addr + 6)
        param = u32(addr + 0x1C)
        off, size = FLOATS[global_idx]
        offs = ', '.join(fmt(v) for v in off)
        sizs = ', '.join(fmt(v) for v in size)
        out.append('    BgObjName_t(0x%X, 0x%X, 0x%X, mVec3_c(%s), mVec2_c(%s), 0x%X),' %
                   (unit, nm, flag, offs, sizs, param))
        global_idx += 1
    out.append('};')
    out.append('')

out.append('static BgObjName_t *l_rail_list[5] = {')
out.append('    l_object_name, l_Pa3_rail, l_Pa3_rail, l_Pa3_daishizen, l_Pa3_MG_house_ami_rail')
out.append('};')

with open(os.path.join(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17',
                       'arrays_generated.txt'), 'w') as fh:
    fh.write('\n'.join(out))
print('\n'.join(out[:30]))
print('...')
print('total elements: %d' % global_idx)
