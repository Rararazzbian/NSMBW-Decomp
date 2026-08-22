"""Byte-compare the draft's .rodata/.data against retail's.

This module is built with `-sdata2 0`, so it has ZERO `@sda21` pool references
(verified: poolcheck.py finds 0 across 38 paired functions).  Every float
constant therefore lives in a named `.rodata`/`.data` object reached by
`lis`/`addi` -- and a relocation's address field is zeroed in both
disassemblies exactly like a pool offset.  So the wrong-constant hole that
poolcheck.py closes for the DOL is closed HERE by comparing the section bytes
themselves, which is strictly stronger: it checks every constant in the unit at
once, not just the ones a matched function happens to load.
"""
import os, sys, struct

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
HERE = os.path.join(ROOT, 'wip', 'castle_r2')

def sections(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'\x7fELF'
    shoff = struct.unpack('>I', d[0x20:0x24])[0]
    shent = struct.unpack('>H', d[0x2e:0x30])[0]
    shnum = struct.unpack('>H', d[0x30:0x32])[0]
    shstr = struct.unpack('>H', d[0x32:0x34])[0]
    def hdr(i):
        o = shoff + i * shent
        return struct.unpack('>10I', d[o:o+40])
    stro = hdr(shstr)[4]
    out = {}
    for i in range(shnum):
        h = hdr(i)
        name = d[stro+h[0]:d.index(b'\0', stro+h[0])].decode()
        out[name] = d[h[4]:h[4]+h[5]] if h[1] != 8 else b'\0' * h[5]
    return out

draft = sections(sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, 'draft.o'))
tgt_ro = sections(os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_03_000086E8_rodata.o'))
tgt_da = sections(os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_04_00043FD0_data.o'))

ok = True
for label, dsec, tsecs in (('.rodata', '.rodata', tgt_ro), ('.data', '.data', tgt_da)):
    a = draft.get(dsec, b'')
    b = tsecs.get(dsec, b'')
    print('%-9s draft=0x%-5x retail=0x%-5x  %s' % (label, len(a), len(b),
          'IDENTICAL' if a == b else 'DIFFERS'))
    if a != b:
        ok = False
        n = max(len(a), len(b))
        for i in range(0, n, 4):
            wa = a[i:i+4]; wb = b[i:i+4]
            if wa != wb:
                fa = struct.unpack('>f', wa)[0] if len(wa) == 4 else None
                fb = struct.unpack('>f', wb)[0] if len(wb) == 4 else None
                print('   +0x%04x  draft %s (%s)   retail %s (%s)'
                      % (i, wa.hex(), fa, wb.hex(), fb))
print('\nCONSTANTS ' + ('CLEAN' if ok else 'MISMATCH'))
