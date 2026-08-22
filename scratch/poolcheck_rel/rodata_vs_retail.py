"""Coverage-complete pool check for LANDED REL units.

The poolcheck sweep can only run where `bin/dtkspl/<mod>/obj` still holds a
retail split object for the range -- and dtk does not emit one for a range that
is already claimed by a slice, so the units landed longest ago have no retail
disassembly to compare against at all.

This closes that gap from the other end, and more strongly. For a landed unit the
literal pool IS its `.rodata` claim. So take the `.rodata` bytes out of the object
the build produced and compare them, byte for byte, against `original/<mod>.rel`
at the offsets the slice claims. If they are equal then every constant that unit
can possibly reference is retail's own constant, whatever the disassembly says.

`.text` is compared the same way as a cross-check on the premise: if `.text` is
equal too then the instruction that reaches into the pool has retail's own
relocation, so it reaches retail's own entry.
"""
import json
import glob
import os
import struct
import sys

ROOT = os.path.abspath('.')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import pool


def obj_sections(path):
    d = open(path, 'rb').read()
    shoff = struct.unpack('>I', d[0x20:0x24])[0]
    shent, shnum, shstr = struct.unpack('>HHH', d[0x2E:0x34])

    def sh(i):
        o = shoff + i * shent
        return struct.unpack('>IIIIIIIIII', d[o:o + 40])

    def name_at(tab, x):
        return d[tab + x:d.index(b'\0', tab + x)].decode('utf-8', 'replace')

    sn = sh(shstr)[4]
    out = {}
    for i in range(shnum):
        s = sh(i)
        out[name_at(sn, s[0])] = (s[1], s[4], s[5])   # type, offset, size
    return d, out


rows = []
for sl in sorted(glob.glob(os.path.join(ROOT, 'slices', 'd_*NP.json'))):
    mod = os.path.basename(sl)[:-5]
    img = pool.rel(json.load(open(sl, encoding='utf-8'))['meta']['moduleNum'])
    for entry in json.load(open(sl, encoding='utf-8'))['slices']:
        src = entry['source']
        obj = os.path.join(ROOT, 'bin', 'compiled', mod,
                           os.path.splitext(src)[0].replace('/', os.sep) + '.o')
        if not os.path.exists(obj):
            rows.append((mod, src, 'NO OBJECT', '', ''))
            continue
        d, secs = obj_sections(obj)
        line = {}
        for sec in ('.text', '.rodata', '.data'):
            rng = entry.get('memoryRanges', {}).get(sec)
            if not rng:
                line[sec] = '-'
                continue
            lo, hi = (int(x, 16) for x in rng.split('-'))
            want = img.read(sec, lo, hi - lo)
            if want is None:
                line[sec] = 'REL READ FAIL'
                continue
            if sec not in secs:
                line[sec] = 'NO SECTION IN OBJ'
                continue
            _t, off, size = secs[sec]
            # Compare only bytes the object actually HAS. A claim is rounded up
            # to the section's 8-byte alignment, so it is routinely 4 bytes
            # longer than the object's section; reading past the section end
            # reads the next section's bytes out of the ELF, which is not a
            # comparison of anything.
            n = min(size, hi - lo)
            got, want = d[off:off + n], want[:n]
            tail = ' (+%#x claim padding not compared)' % ((hi - lo) - n) if n < hi - lo else ''
            if got == want:
                line[sec] = 'EQUAL %#x%s' % (n, tail)
            else:
                nd = sum(1 for a, b in zip(got, want) if a != b)
                first = next(i for i, (a, b) in enumerate(zip(got, want)) if a != b)
                line[sec] = 'DIFFERS %d/%#x bytes, first at +%#x' % (nd, n, first)
        rows.append((mod, src, line['.text'], line['.rodata'], line['.data']))

print('%-14s %-42s %-26s %-30s %s' % ('module', 'source', '.text', '.rodata', '.data'))
bad = 0
for mod, src, t, r, dd in rows:
    flag = ''
    if 'DIFFERS' in (r or '') or 'DIFFERS' in (t or ''):
        flag = '   <<<<'
        bad += 1
    print('%-14s %-42s %-26s %-30s %s%s' % (mod, src, t, r, dd, flag))
print('\n%d row(s), %d with a DIFFERING .text or .rodata' % (len(rows), bad))
