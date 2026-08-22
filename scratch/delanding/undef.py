import struct, sys, glob, os

def sections(data):
    (e_shoff,) = struct.unpack_from('>I', data, 0x20)
    e_shentsize, e_shnum, e_shstrndx = struct.unpack_from('>HHH', data, 0x2E)
    secs = []
    for i in range(e_shnum):
        o = e_shoff + i * e_shentsize
        name, typ, flags, addr, off, size, link, info, align, entsize = \
            struct.unpack_from('>IIIIIIIIII', data, o)
        secs.append(dict(name=name, type=typ, off=off, size=size, link=link,
                         entsize=entsize))
    shstr = secs[e_shstrndx]
    for s in secs:
        end = data.index(b'\0', shstr['off'] + s['name'])
        s['sname'] = data[shstr['off'] + s['name']:end].decode()
    return secs

BIND = {0: 'LOCAL', 1: 'GLOBAL', 2: 'WEAK'}

for path in sys.argv[1:]:
    data = open(path, 'rb').read()
    secs = sections(data)
    symtab = [s for s in secs if s['sname'] == '.symtab'][0]
    strtab = secs[symtab['link']]
    n = symtab['size'] // 16
    undef = []
    for i in range(n):
        o = symtab['off'] + i * 16
        st_name, st_value, st_size, st_info, st_other, st_shndx = \
            struct.unpack_from('>IIIBBH', data, o)
        if st_shndx != 0 or st_name == 0:
            continue
        end = data.index(b'\0', strtab['off'] + st_name)
        nm = data[strtab['off'] + st_name:end].decode()
        undef.append((BIND.get(st_info >> 4, '?'), nm))
    print('=' * 70)
    print('%s -- %d UNDEFINED symbols' % (os.path.basename(path), len(undef)))
    for b, nm in sorted(undef):
        print('   %-7s %s' % (b, nm))
