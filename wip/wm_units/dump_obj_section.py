"""Dump a section's raw bytes from a compiled ELF object, with float/ASCII decoding.

Why this exists
---------------
Comparing a draft's emitted `.rodata`/`.data` against the retail bytes word by
word has been the decisive step on several units -- a pool that is the right
SIZE but the wrong COMPOSITION is invisible from the functions themselves, and
one dead or missing word has held three functions open at a time.

But there was no way to do it: no `elftools`, no `objdump` in this toolchain.
An agent correctly reported the check as "not attempted" rather than faking a
result. This closes that gap.

ELF parsing here is deliberately minimal and self-contained -- big-endian 32-bit
PowerPC objects only, which is all this project produces.

Usage
-----
    python wip/wm_units/dump_obj_section.py <object.o> [section] [start] [end]

e.g.
    python wip/wm_units/dump_obj_section.py wip/wm_units/agent_killer/draft.o .rodata
    python wip/wm_units/dump_obj_section.py draft.o .data 0x0 0x80

Compare against the retail bytes with the REL reader: `.text` lives at file
offset 0xF0, and the other sections' offsets come from the section table at
0x10 (see `wip/wm_units/profile_map.py`).
"""
import os
import struct
import sys


def sections(path):
    """[(name, offset, size)] for every section in a big-endian 32-bit ELF."""
    b = open(path, 'rb').read()
    if b[:4] != b'\x7fELF':
        raise SystemExit('not an ELF file: %s' % path)
    e_shoff, = struct.unpack_from('>I', b, 0x20)
    e_shentsize, e_shnum, e_shstrndx = struct.unpack_from('>HHH', b, 0x2E)

    def entry(i):
        off = e_shoff + i * e_shentsize
        sh_name, _type, _flags, _addr, sh_offset, sh_size = struct.unpack_from('>IIIIII', b, off)
        return sh_name, sh_offset, sh_size

    _n, str_off, _s = entry(e_shstrndx)
    out = []
    for i in range(e_shnum):
        sh_name, sh_offset, sh_size = entry(i)
        end = b.index(b'\0', str_off + sh_name)
        out.append((b[str_off + sh_name:end].decode(), sh_offset, sh_size))
    return b, out


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    path = sys.argv[1]
    want = sys.argv[2] if len(sys.argv) > 2 else None
    lo = int(sys.argv[3], 0) if len(sys.argv) > 3 else 0
    hi = int(sys.argv[4], 0) if len(sys.argv) > 4 else None

    b, secs = sections(path)
    if want is None:
        print('%-24s %10s %10s' % ('section', 'offset', 'size'))
        for name, off, size in secs:
            if size:
                print('%-24s %#10x %#10x' % (name, off, size))
        return 0

    for name, off, size in secs:
        if name != want:
            continue
        end = size if hi is None else min(hi, size)
        print('%s  (%#x bytes)' % (name, size))
        for a in range(lo, end, 4):
            w, = struct.unpack_from('>I', b, off + a)
            f, = struct.unpack('>f', struct.pack('>I', w))
            raw = b[off + a:off + a + 4]
            txt = ''.join(chr(c) if 32 <= c < 127 else '.' for c in raw)
            # A relocated word reads as zero in the file -- see HANDOFF.md.
            print('+0x%04x  %08x  %-6s %g' % (a, w, txt, f))
        return 0

    raise SystemExit('no section %r in %s' % (want, path))


if __name__ == '__main__':
    raise SystemExit(main())
