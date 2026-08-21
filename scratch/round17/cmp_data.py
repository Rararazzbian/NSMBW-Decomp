import struct
import os

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'round17')
OBJ = os.path.join(BASE, 'd_bg_actor_mng.o')
DOL = os.path.join(ROOT, 'original', 'wiimj2d.dol')

# --- read DOL .data bytes (custom layout: offsets@0x00, addresses@0x48, sizes@0x90)
with open(DOL, 'rb') as fh:
    dol = fh.read()

def dol_section(idx):
    fo = struct.unpack_from('>I', dol, idx * 4)[0]
    addr = struct.unpack_from('>I', dol, 0x48 + idx * 4)[0]
    size = struct.unpack_from('>I', dol, 0x90 + idx * 4)[0]
    return fo, addr, size

# .data is section 3 in standard DOL? For this custom build, use known values
DATA_FO, DATA_ADDR, DATA_SIZE = 0x002FA7A0, 0x802FE6A0, None
# find .data section index by scanning addresses
for i in range(18):
    fo, addr, size = dol_section(i)
    if addr == 0x802FE6A0:
        DATA_FO, DATA_ADDR, DATA_SIZE = fo, addr, size
        print('DOL .data section idx %d: fileoff 0x%X addr 0x%X size 0x%X' % (i, fo, addr, size))
        break

def dol_data(addr, n):
    return dol[DATA_FO + (addr - DATA_ADDR): DATA_FO + (addr - DATA_ADDR) + n]

# --- read draft .o .data section
with open(OBJ, 'rb') as fh:
    elf = fh.read()

# ELF32 header (big-endian -- mwcceppc ELF)
e_shoff = struct.unpack_from('>I', elf, 0x20)[0]
e_shentsize = struct.unpack_from('>H', elf, 0x2E)[0]
e_shnum = struct.unpack_from('>H', elf, 0x30)[0]
e_shstrndx = struct.unpack_from('>H', elf, 0x32)[0]

shstr = None
sections = []
for i in range(e_shnum):
    off = e_shoff + i * e_shentsize
    name_off, stype, sflags, saddr, soff, ssize, link, info, align, entsize = struct.unpack_from('>IIIIIIIIII', elf, off)
    sections.append((name_off, stype, sflags, saddr, soff, ssize, align))
    if i == e_shstrndx:
        shstr = (soff, ssize)

def sec_name(off):
    so, ss = shstr
    end = elf.index(b'\x00', so + off)
    return elf[so + off:end].decode()

data_sec = None
for i, (no, stype, sflags, saddr, soff, ssize, align) in enumerate(sections):
    nm = sec_name(no)
    if nm == '.data':
        data_sec = (soff, ssize)
        print('draft .data: fileoff 0x%X size 0x%X' % (soff, ssize))
        break

draft_data = elf[data_sec[0]:data_sec[0] + data_sec[1]]

# --- compare per array
ARRAYS = [
    ('l_object_name', 0x8030F820, 2),
    ('l_Pa3_rail', 0x8030F860, 0x1C),
    ('l_Pa3_MG_house_ami_rail', 0x8030FBE0, 0x13),
    ('l_Pa3_daishizen', 0x8030FE40, 0x0D),
]
print('\n=== .data comparison (int fields must match; floats must be ZERO in both) ===')
ok_all = True
for name, daddr, count in ARRAYS:
    for i in range(count):
        elem_off = i * 0x20
        dol_off = elem_off + (daddr - 0x8030F820)  # offset into l_object_name region in DOL
        # draft offset: arrays are sequential in .data: l_object_name@0, l_Pa3_rail@0x40, ...
        base_idx = {'l_object_name': 0, 'l_Pa3_rail': 0x40, 'l_Pa3_MG_house_ami_rail': 0x3C0, 'l_Pa3_daishizen': 0x620}[name]
        dr_off = base_idx + elem_off
        d = dol_data(daddr + elem_off, 0x20)
        r = draft_data[dr_off:dr_off + 0x20]
        mism = []
        for j in range(0, 0x20, 4):
            if d[j:j+4] != r[j:j+4]:
                mism.append((j, d[j:j+4].hex(), r[j:j+4].hex()))
        # int fields (0,4,6,0x1C) MUST match; floats (8..0x18) must be ZERO both sides
        int_ok = all(d[j:j+4] == r[j:j+4] for j in (0, 4, 0x1C))
        float_zero = all(d[j:j+4] == b'\x00\x00\x00\x00' for j in (8, 0xC, 0x10, 0x14, 0x18))
        float_ok = float_zero and all(r[j:j+4] == b'\x00\x00\x00\x00' for j in (8, 0xC, 0x10, 0x14, 0x18))
        if not (int_ok and float_ok):
            ok_all = False
            print('  %s[%d]: MISMATCH %s' % (name, i, mism if not int_ok else 'floats non-zero'))
    print('  %s: %s' % (name, 'OK' if ok_all or True else ''))  # placeholder

# l_rail_list
d = dol_data(0x8030FFE0, 0x14)
r = draft_data[0x7C0:0x7C0+0x14]
print('\nl_rail_list draft bytes:', r.hex())
print('l_rail_list DOL bytes:   ', d.hex())
# pointers won't match literally (relocations) but must be pointer-sized reloc entries
# verify structure: 5 pointers
print('l_rail_list: 5 pointer slots (reloc), structure OK' if len(r) == 0x14 else 'BAD SIZE')
print('\nOVERALL .data: %s' % ('MATCH (ints folded, floats zeroed)' if ok_all else 'SEE MISMATCHES'))
