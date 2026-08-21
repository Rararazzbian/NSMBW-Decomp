import struct
import os

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'round17')
OBJ = os.path.join(BASE, 'd_bg_actor_mng.o')
DOL = os.path.join(ROOT, 'original', 'wiimj2d.dol')

# DOL sections
with open(DOL, 'rb') as fh:
    dol = fh.read()

def dol_section(idx):
    fo = struct.unpack_from('>I', dol, idx * 4)[0]
    addr = struct.unpack_from('>I', dol, 0x48 + idx * 4)[0]
    size = struct.unpack_from('>I', dol, 0x90 + idx * 4)[0]
    return fo, addr, size

# target .bss region: 4 nodes @0x80356230, 0xC each
fo, addr, size = dol_section(12)  # .data
# find .bss section (bss usually has addr after data)
bss_nodes = []
for i in range(18):
    f2, a2, s2 = dol_section(i)
    if a2 == 0x80356230:
        print('DOL .bss section idx %d @0x%X size 0x%X' % (i, a2, s2))
        for k in range(4):
            node = dol[f2 + (0x80356230 + k*0xC - a2): f2 + (0x80356230 + k*0xC - a2) + 0xC]
            bss_nodes.append(node.hex())
        break
print('target .bss register nodes (4x0xC):')
for n in bss_nodes:
    print('  %s' % n)

# draft .o .bss section
with open(OBJ, 'rb') as fh:
    elf = fh.read()
e_shoff = struct.unpack_from('>I', elf, 0x20)[0]
e_shentsize = struct.unpack_from('>H', elf, 0x2E)[0]
e_shnum = struct.unpack_from('>H', elf, 0x30)[0]
e_shstrndx = struct.unpack_from('>H', elf, 0x32)[0]
shstr = None
for i in range(e_shnum):
    off = e_shoff + i * e_shentsize
    no, stype, sflags, saddr, soff, ssize, link, info, align, entsize = struct.unpack_from('>IIIIIIIIII', elf, off)
    if i == e_shstrndx:
        shstr = (soff, ssize)
def sec_name(off):
    so, ss = shstr
    end = elf.index(b'\x00', so + off)
    return elf[so + off:end].decode()
for i in range(e_shnum):
    off = e_shoff + i * e_shentsize
    no, stype, sflags, saddr, soff, ssize, link, info, align, entsize = struct.unpack_from('>IIIIIIIIII', elf, off)
    nm = sec_name(no)
    if nm in ('.bss', '.sbss', '.ctors'):
        print('draft %s: size 0x%X align %d' % (nm, ssize, align))

# draft .ctors content
for i in range(e_shnum):
    off = e_shoff + i * e_shentsize
    no, stype, sflags, saddr, soff, ssize, link, info, align, entsize = struct.unpack_from('>IIIIIIIIII', elf, off)
    nm = sec_name(no)
    if nm == '.ctors':
        print('draft .ctors bytes:', elf[soff:soff+ssize].hex())

# target .ctors 0x802EDD94 -> 0x8007EC20
fo, addr, size = dol_section(2)  # .ctors typically section 2
print('DOL .ctors section idx2 @0x%X size 0x%X' % (addr, size))
