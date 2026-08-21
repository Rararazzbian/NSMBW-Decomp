import struct
import os

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
OBJ = os.path.join(BASE, 'd_bg_actor_mng.o')

elf = open(OBJ, 'rb').read()
sh = struct.unpack_from('>I', elf, 0x20)[0]
ss = struct.unpack_from('>H', elf, 0x2E)[0]
sn = struct.unpack_from('>H', elf, 0x30)[0]
si = struct.unpack_from('>H', elf, 0x32)[0]

secs = []
for i in range(sn):
    o = sh + i * ss
    no, ty, fl, ad, fo, sz, ln, inf, al, es = struct.unpack_from('>IIIIIIIIII', elf, o)
    secs.append((no, ty, fl, ad, fo, sz, al))

sstr = (secs[si][4], secs[si][5])

def nm(o):
    so, sz = sstr
    e = elf.index(b'\x00', so + o)
    return elf[so + o:e].decode()

names = [nm(s[0]) for s in secs]
for no, ty, fl, ad, fo, sz, al in secs:
    n = nm(no)
    if n in ('.bss', '.sbss', '.ctors', '.sdata2', '.data'):
        print('%s: size 0x%X align %d' % (n, sz, al))

# .sdata2 bytes
idx = names.index('.sdata2')
fo, sz = secs[idx][4], secs[idx][5]
print('sdata2 bytes:', elf[fo:fo + sz].hex())

# symbols in .sbss
print('\n-- sbss/bss symbols --')
for no, ty, fl, ad, fo, sz, al in secs:
    n = nm(no)
    if n in ('.sbss', '.bss'):
        print(n, 'size', hex(sz))
