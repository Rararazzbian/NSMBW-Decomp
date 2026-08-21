import re
import struct
import os

# Extract per-element float ctor args from __sinit__ disasm.
# __sinit builds each BgObjName_t element:
#   temp mVec3 on stack -> copy to element+0x08 (offset)
#   temp mVec2 on stack -> copy to element+0x14 (size)
# r6 points at the ARRAY base (absolute from r31), and elements are written
# at base + i*0x20 + {0x8..0x18, 0x14..0x18}. So element = (base+off)>>5,
# field = (base+off) & 0x1F.

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
SRC = os.path.join(BASE, 'target_sinit_d_bg_actor_mn.txt')

with open(os.path.join(BASE, 'sdata2_8042C130.bin'), 'rb') as fh:
    raw = fh.read()
pool = {}
for i in range(0, len(raw), 4):
    pool[0x8042C130 + i] = struct.unpack_from('>f', raw, i)[0]

# r31 = array base (l_object_name @ .data). Track its value from addi r31.
ADD_R31 = re.compile(r'addi r31, r31, (0x[0-9A-Fa-f]+)')
ADD_R6_FROM_R31 = re.compile(r'addi r6, r31, (0x[0-9A-Fa-f]+)')
ADD_R6_INC = re.compile(r'addi r6, r6, (0x[0-9A-Fa-f]+)')
CONST_LOAD = re.compile(r'\b(lfs|lfd)\s+f(\d+),\s+"@\d+_8042C([0-9A-Fa-f]{3})"')
STFS_R6 = re.compile(r'\bstfs\s+f(\d+),\s+(0x[0-9A-Fa-f]+)\(r6\)')

r31 = 0
r6 = None
fpr = {}
elements = {}  # elem_index -> {'off':[..], 'size':[..]}
with open(SRC, encoding='utf-8', errors='replace') as fh:
    for line in fh:
        m = ADD_R6_FROM_R31.search(line)
        if m:
            r6 = r31 + int(m.group(1), 16)
            continue
        m = ADD_R6_INC.search(line)
        if m:
            if r6 is not None:
                r6 += int(m.group(1), 16)
            continue
        m = ADD_R31.search(line)
        if m:
            r31 = int(m.group(1), 16)
            continue
        m = CONST_LOAD.search(line)
        if m:
            caddr = 0x8042C000 + int(m.group(3), 16)
            if m.group(1) == 'lfd':
                # doubles live at aligned addresses; reinterpret 8 bytes
                fpr[int(m.group(2))] = struct.unpack_from('>d', raw, caddr - 0x8042C130)[0]
            else:
                fpr[int(m.group(2))] = pool.get(caddr)
            continue
        m = STFS_R6.search(line)
        if m and r6 is not None:
            reg = int(m.group(1))
            abs_off = r6 + int(m.group(2), 16)
            idx = abs_off >> 5
            field = abs_off & 0x1F
            val = fpr.get(reg)
            if val is None:
                continue
            e = elements.setdefault(idx, {'off': [None, None, None], 'size': [None, None]})
            if field == 0x8:
                e['off'][0] = val
            elif field == 0xC:
                e['off'][1] = val
            elif field == 0x10:
                e['off'][2] = val
            elif field == 0x14:
                e['size'][0] = val
            elif field == 0x18:
                e['size'][1] = val

# group by array: [0..1] l_object_name, [2..29] l_Pa3_rail,
# [30..42] l_Pa3_MG_house_ami_rail, [43..55] l_Pa3_daishizen
print('total elements: %d' % len(elements))
ranges = [('l_object_name', 0, 2), ('l_Pa3_rail', 2, 28),
          ('l_Pa3_MG_house_ami_rail', 30, 19), ('l_Pa3_daishizen', 49, 13)]
for name, start, count in ranges:
    print('\n=== %s (global %d..%d) ===' % (name, start, start + count - 1))
    for i in range(start, start + count):
        e = elements.get(i)
        if not e:
            print('  [%02d] (no runtime float writes)' % i)
            continue
        off = e['off']
        size = e['size']
        print('  [%02d] off=(%s,%s,%s) size=(%s,%s)' %
              (i, off[0], off[1], off[2], size[0], size[1]))
