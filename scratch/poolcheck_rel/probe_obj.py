import struct, sys, re
p = sys.argv[1]
d = open(p,'rb').read()
shoff = struct.unpack('>I', d[0x20:0x24])[0]
shent, shnum, shstr = struct.unpack('>HHH', d[0x2E:0x34])
def sh(i):
    o = shoff + i*shent
    return struct.unpack('>IIIIIIIIII', d[o:o+40])
def name_at(tab,x):
    e = d.index(b'\0', tab+x); return d[tab+x:e].decode('utf-8','replace')
sn = sh(shstr)[4]
names = [name_at(sn, sh(i)[0]) for i in range(shnum)]
for i,n in enumerate(names):
    s = sh(i)
    print(f'{i:3d} {n:20s} type={s[1]:2d} off=0x{s[4]:06X} size=0x{s[5]:X} link={s[6]} info={s[7]} entsz={s[9]}')
idx = {n:i for i,n in enumerate(names)}
sym = sh(idx['.symtab']); strtab = sh(idx['.strtab'])[4]
syms=[]
for k in range(sym[5]//16):
    o = sym[4]+k*16
    st_name, st_val, st_size, st_info, st_other, st_shndx = struct.unpack('>IIIBBH', d[o:o+16])
    syms.append((name_at(strtab, st_name), st_val, st_size, st_info, st_shndx))
print('--- symbols with empty names or section syms ---')
for k,(n,v,sz,inf,shx) in enumerate(syms):
    if (inf & 0xf) == 3 or n=='':
        print(f'  sym[{k}] name={n!r} val=0x{v:X} shndx={shx} ({names[shx] if shx<shnum else "?"}) type={inf&0xf}')
# relocations for .text
for i,n in enumerate(names):
    if n in ('.rela.text',):
        s = sh(i)
        print(f'--- {n}: {s[5]//12} entries ---')
        for k in range(s[5]//12):
            o = s[4]+k*12
            r_off, r_info, r_add = struct.unpack('>IIi', d[o:o+12])
            si, rt = r_info>>8, r_info&0xff
            print(f'  0x{r_off:06X} type={rt:3d} sym={si} {syms[si][0]!r} shndx={syms[si][4]} val=0x{syms[si][1]:X} addend=0x{r_add:X}')
