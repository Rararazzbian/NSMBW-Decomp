"""Index bin/dtkspl/obj/*_text.o retail split objects by absolute VA range."""
import os
import re
import struct

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
OBJDIR = os.path.join(ROOT, 'bin', 'dtkspl', 'obj')

NAME_ADDR = re.compile(r'auto_\d+_([0-9A-Fa-f]{8})_text\.o$')


def text_size(path):
    d = open(path, 'rb').read()
    shoff = struct.unpack('>I', d[0x20:0x24])[0]
    shent, shnum, shstrndx = struct.unpack('>HHH', d[0x2E:0x34])

    def sh(i):
        o = shoff + i * shent
        return struct.unpack('>IIIIIIIIII', d[o:o + 40])

    def name_at(tab, x):
        end = d.index(b'\0', tab + x)
        return d[tab + x:end].decode('utf-8', 'replace')

    sn = sh(shstrndx)[4]
    for i in range(shnum):
        s = sh(i)
        if name_at(sn, s[0]) == '.text':
            return s[5]
    return 0


def build_index(objdir=OBJDIR):
    """[(lo, hi, path), ...] sorted by lo, for every filename-addressed
    _text.o. Files without an address in their name (auto_sinit_*) are
    returned separately -- report them as a coverage gap."""
    out, unnamed = [], []
    for fn in os.listdir(objdir):
        if not fn.endswith('_text.o'):
            continue
        m = NAME_ADDR.search(fn)
        p = os.path.join(objdir, fn)
        if not m:
            unnamed.append(p)
            continue
        lo = int(m.group(1), 16)
        sz = text_size(p)
        out.append((lo, lo + sz, p))
    out.sort()
    return out, unnamed


def overlapping(index, lo, hi):
    return [p for (a, b, p) in index if a < hi and b > lo]


if __name__ == '__main__':
    idx, unnamed = build_index()
    print(f'{len(idx)} addressed split objects, {len(unnamed)} unnamed (no VA in filename)')
    total = sum(b - a for a, b, _ in idx)
    print(f'total .text bytes covered: 0x{total:X}')
    # sanity: d_2d.cpp .text 0x10-0xd90 relative to .text base 0x80006780
    lo, hi = 0x80006780 + 0x10, 0x80006780 + 0xd90
    hit = overlapping(idx, lo, hi)
    print(f'd_2d.cpp range 0x{lo:X}-0x{hi:X}: {len(hit)} split object(s)')
    for p in hit:
        print('  ', p)
