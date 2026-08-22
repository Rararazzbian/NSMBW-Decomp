import struct, json, os, glob
ROOT='.'
mods={}
for p in glob.glob('slices/*.json'):
    j=json.load(open(p)); m=j['meta']
    if m.get('type')!='REL': continue
    mods[m['moduleNum']]=(m['fileName'], {k:v['index'] for k,v in m['sections'].items()})
print({k:v[0] for k,v in mods.items()})

def rel_sections(path):
    d=open(path,'rb').read()
    (index,_,_,section_count,section_info_offset)=struct.unpack('>5I', d[0:20])
    out=[]
    for i in range(section_count):
        o=section_info_offset+i*8
        off_flags, ln = struct.unpack('>II', d[o:o+8])
        out.append((off_flags & ~3, bool(off_flags&1), ln))
    return d, out

d, secs = rel_sections('original/d_basesNP.rel')
for i,(o,ex,l) in enumerate(secs):
    print(i, hex(o), ex, hex(l))
ro = secs[mods[2][1]['.rodata']]
print('rodata file off', hex(ro[0]), 'len', hex(ro[2]))
for off in (0x87B0,0x87C0,0x87D0,0x87D4):
    b=d[ro[0]+off:ro[0]+off+4]
    print(f'  .rodata+0x{off:X} = {b.hex().upper()} -> {struct.unpack(">f",b)[0]!r}')

# draft object .rodata
dd=open('wip/course_r2/draft.o','rb').read()
base=0x17B8
for off in (0x00,0x10,0x20,0x24,0x28):
    b=dd[base+off:base+off+4]
    print(f'  draft .rodata+0x{off:X} = {b.hex().upper()} -> {struct.unpack(">f",b)[0]!r}')
