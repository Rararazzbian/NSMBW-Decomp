"""Compare the draft's .data/.rodata/.ctors RELOCATIONS against retail's.

datacheck.py compares section BYTES, and a relocated word reads as zero on both
sides -- the same blind spot that makes a wrong `bl` target byte-identical.  So
the bytes being equal does not prove the pointers point at the same things.
This resolves each relocation to the symbol it names, on both sides, and lines
them up by offset.
"""
import os, struct, sys

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
HERE = os.path.join(ROOT, 'wip', 'castle_r2')


def elf(path):
    d = open(path, 'rb').read()
    shoff = struct.unpack('>I', d[0x20:0x24])[0]
    shent = struct.unpack('>H', d[0x2e:0x30])[0]
    shnum = struct.unpack('>H', d[0x30:0x32])[0]
    shstr = struct.unpack('>H', d[0x32:0x34])[0]
    hdrs = [struct.unpack('>10I', d[shoff+i*shent:shoff+i*shent+40]) for i in range(shnum)]
    stro = hdrs[shstr][4]
    def nm(off, base):
        return d[base+off:d.index(b'\0', base+off)].decode('utf-8', 'replace')
    names = [nm(h[0], stro) for h in hdrs]
    # symbol table
    si = names.index('.symtab')
    strtab = hdrs[hdrs[si][6]][4]
    syms = []
    off, size = hdrs[si][4], hdrs[si][5]
    for i in range(size // 16):
        o = off + i*16
        n, val, sz, info, other, shndx = struct.unpack('>IIIBBH', d[o:o+16])
        syms.append((nm(n, strtab), val, shndx))
    out = {}
    for i, h in enumerate(hdrs):
        if names[i].startswith('.rela'):
            tgt = names[i][5:]
            rels = []
            for j in range(h[5] // 12):
                o = h[4] + j*12
                r_off, r_info, r_add = struct.unpack('>IIi', d[o:o+12])
                symi = r_info >> 8
                sname, sval, shndx = syms[symi]
                if not sname:  # section symbol -> name it by its section
                    sname = '<' + names[shndx] + '>'
                rels.append((r_off, r_info & 0xff, sname, r_add))
            out[tgt] = sorted(rels)
    return out


draft = elf(os.path.join(HERE, 'draft.o'))
tgt = {}
for f, sec in (('auto_04_00043FD0_data.o', '.data'),
               ('auto_03_000086E8_rodata.o', '.rodata')):
    e = elf(os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', f))
    if sec in e:
        tgt[sec] = e[sec]

# Relocations inside a WEAK symbol are excluded: unreferenced weak symbols are not
# placed by the linker, and the landed, 5/5-verified d_a_wm_cloud.o carries exactly the
# same trailing __vt__13dWmObjActor_c / __vt__Q23m3d8anmChr_c pair. Cut each section at
# the end of the draft's own STRONG symbols.
WEAK_FROM = {'.data': 0x150, '.rodata': 0x1000}

ok = True
for sec in ('.data', '.rodata'):
    cut = WEAK_FROM.get(sec, 1 << 30)
    a = [r for r in draft.get(sec, []) if r[0] < cut]
    b = [r for r in tgt.get(sec, []) if r[0] < cut]
    print('== %s == draft %d relocs, retail %d' % (sec, len(a), len(b)))
    if len(a) != len(b):
        ok = False
    for i in range(max(len(a), len(b))):
        ra = a[i] if i < len(a) else None
        rb = b[i] if i < len(b) else None
        same = ra and rb and ra[0] == rb[0] and ra[1] == rb[1] and ra[3] == rb[3]
        mark = '  ' if same else '**'
        if not same:
            ok = False
        print('  %s off=%-8s type=%-4s retail=%-34s draft=%s'
              % (mark,
                 hex(rb[0]) if rb else hex(ra[0]),
                 (rb or ra)[1],
                 rb[2] if rb else '-',
                 ra[2] if ra else '-'))
print('\nRELOCATIONS ' + ('ALIGNED' if ok else 'MISALIGNED'))
