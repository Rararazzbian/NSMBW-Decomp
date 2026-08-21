import struct
import os

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'

# draft sdata2
elf = open(os.path.join(BASE, 'd_bg_actor_mng.o'), 'rb').read()
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
idx = names.index('.sdata2')
fo, sz = secs[idx][4], secs[idx][5]
raw = elf[fo:fo + sz]
print('draft .sdata2 (%d bytes):' % sz)
for i in range(0, sz, 4):
    w = struct.unpack_from('>I', raw, i)[0]
    f = struct.unpack_from('>f', raw, i)[0]
    print('  +0x%02X: 0x%08X = %g' % (i, w, f))

# target sdata2 dump
dol = open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol', 'rb').read()
fo2 = 0x002FA7A0 + (0x8042C130 - 0x802FE6A0)
raw2 = dol[fo2:fo2 + 0x38]
print('\ntarget .sdata2 (0x38 bytes):')
for i in range(0, 0x38, 4):
    w = struct.unpack_from('>I', raw2, i)[0]
    f = struct.unpack_from('>f', raw2, i)[0]
    print('  +0x%02X: 0x%08X = %g' % (i, w, f))
