"""Decode a literal-pool constant out of a retail binary, by address or by symbol.

Two separate agents have now been blocked on "I could not determine the value of
`@54951_8042CB1C` from the disassembly", and one of them papered over it by
brute-forcing constants until the bytes matched -- which always succeeds, because
`lfs f1, "@54951_8042CB1C"@sda21(r0)` assembles to `C0 20 00 00` with the offset
field ZEROED. A wrong constant is byte-identical to the right one. The pattern
can never tell you the value; only the binary can.

    python pool.py 0x8042CB1C            # DOL, by virtual address
    python pool.py @54951_8042CB1C       # the VA is embedded in the symbol name
    python pool.py 0x8042CB1C 0x8042CB48 # several at once
    python pool.py lbl_2_rodata_87D0     # a REL label: module 2, .rodata+0x87D0

dtk's pool symbol names carry the address after the underscore, so you can paste
one straight out of a disassembly listing.

REL binaries
------------
The four `.rel` modules are NOT address spaces -- they are relocatable, so a
constant has a SECTION and an OFFSET, never a VA. dtk names them
`lbl_<module>_<section>_<offset>`, which is self-describing, and `RelImage`
below turns that straight into bytes out of `original/<module>.rel`.

This matters because the RELs are compiled with `-sdata 0 -sdata2 0`, so they
have NO `@sda21` addressing at all. Anything that only recognises the DOL's
`@sda21` form sees nothing whatsoever in a REL -- see poolcheck.py's header.
"""
import json
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DOL = os.path.join(ROOT, 'original', 'wiimj2d.dol')
SLICES = os.path.join(ROOT, 'slices')
ORIGINAL = os.path.join(ROOT, 'original')


def load(path=DOL):
    """Return (data, [(file_offset, virtual_address, size), ...]).

    The DOL header is a fixed table: 7 text then 11 data sections, with offsets
    at 0x00, addresses at 0x48 and sizes at 0x90. Sections with size 0 are unused
    slots and are dropped.
    """
    with open(path, 'rb') as f:
        d = f.read()
    offs = struct.unpack('>18I', d[0x00:0x48])
    adrs = struct.unpack('>18I', d[0x48:0x90])
    szs = struct.unpack('>18I', d[0x90:0xD8])
    return d, [(o, a, s) for o, a, s in zip(offs, adrs, szs) if s]


def va_to_off(va, sections):
    for o, a, s in sections:
        if a <= va < a + s:
            return o + (va - a)
    return None


def read(va, path=DOL):
    """Decode `va` as both a 4-byte float and an 8-byte double.

    Returns a dict, or None if the address is not inside any section. Both
    readings are given because which one is correct depends on whether the
    instruction was `lfs` or `lfd` -- and that distinction is exactly what tells
    you whether the original source wrote a trailing `f`.
    """
    d, sections = load(path)
    off = va_to_off(va, sections)
    if off is None:
        return None
    b4, b8 = d[off:off + 4], d[off:off + 8]
    out = {'va': va, 'off': off, 'bytes4': b4.hex().upper(),
           'f32': struct.unpack('>f', b4)[0]}
    if len(b8) == 8:
        out['bytes8'] = b8.hex().upper()
        out['f64'] = struct.unpack('>d', b8)[0]
    return out


# ------------------------------------------------------------------ REL images

# `lbl_2_rodata_87B0` -- module number, section NAME, section-relative offset.
# The section name is spelled without its leading dot in the label.
REL_LABEL = re.compile(r'^lbl_(\d+)_(text|ctors|dtors|rodata|data|bss)_'
                       r'([0-9A-Fa-f]+)$')

_MODULE_META = None


def module_meta():
    """{moduleNum: (rel filename, {section name: section index})} for every REL.

    Read from the slice files rather than hardcoded, for the same reason
    harness.flags_for() reads the compiler flags from them: a duplicated copy of
    the section table drifts, and a wrong section index reads the wrong bytes and
    reports a wrong constant, which is precisely the failure this module exists
    to prevent.
    """
    global _MODULE_META
    if _MODULE_META is None:
        _MODULE_META = {}
        for name in os.listdir(SLICES):
            if not name.endswith('.json'):
                continue
            with open(os.path.join(SLICES, name), encoding='utf-8') as fh:
                meta = json.load(fh)['meta']
            if meta.get('type') != 'REL':
                continue
            _MODULE_META[int(meta['moduleNum'])] = (
                meta['fileName'], {k: v['index'] for k, v in meta['sections'].items()})
    return _MODULE_META


class RelImage:
    """Section-relative reader over one `original/<module>.rel`.

    The REL header holds a section table of (offset|flags, length) pairs; a zero
    offset means `.bss`, which has no bytes on disk. Nothing here relocates
    anything -- we only ever read constant DATA, never code.
    """

    def __init__(self, module_num):
        meta = module_meta().get(int(module_num))
        if meta is None:
            raise KeyError('no REL slice file declares moduleNum %s -- known: %s'
                           % (module_num, sorted(module_meta())))
        self.module_num = int(module_num)
        self.filename, self.section_index = meta
        with open(os.path.join(ORIGINAL, self.filename), 'rb') as fh:
            self.data = fh.read()
        count, info_off = struct.unpack('>II', self.data[0x0C:0x14])
        self.sections = []
        for i in range(count):
            o = info_off + i * 8
            flags, length = struct.unpack('>II', self.data[o:o + 8])
            self.sections.append((flags & ~3, length))

    def read(self, section, offset, size):
        """`section` may be given with or without its leading dot."""
        if not section.startswith('.'):
            section = '.' + section
        idx = self.section_index.get(section)
        if idx is None or idx >= len(self.sections):
            return None
        base, length = self.sections[idx]
        if base == 0:                       # .bss -- no bytes in the file
            return None
        if offset < 0 or offset + size > length:
            return None
        return self.data[base + offset:base + offset + size]


_REL_CACHE = {}


def rel(module_num):
    if module_num not in _REL_CACHE:
        _REL_CACHE[module_num] = RelImage(module_num)
    return _REL_CACHE[module_num]


def parse_arg(a):
    """Accept a bare address, or a dtk pool symbol with the VA in its name."""
    m = re.search(r'([0-9A-Fa-f]{8})\b', a.replace('0x', '', 1) if a.lower().startswith('0x') else a)
    if a.lower().startswith('0x'):
        return int(a, 16)
    if m:
        return int(m.group(1), 16)
    raise ValueError(f'cannot read an address out of {a!r}')


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    for a in sys.argv[1:]:
        m = REL_LABEL.match(a.strip('"'))
        if m:
            mod, sec, off = int(m.group(1)), m.group(2), int(m.group(3), 16)
            try:
                img = rel(mod)
            except KeyError as exc:
                print(f'{a}: {exc.args[0]}')
                continue
            b8 = img.read(sec, off, 8) or img.read(sec, off, 4)
            if b8 is None:
                print(f'{a}: not inside {img.filename} .{sec}')
                continue
            line = (f'{a:24s} {img.filename} .{sec}+0x{off:X}  '
                    f'{b8[:4].hex().upper()} -> f32 {struct.unpack(">f", b8[:4])[0]!r}')
            if len(b8) == 8:
                line += (f'   |  {b8.hex().upper()} -> f64 '
                         f'{struct.unpack(">d", b8)[0]!r}')
            print(line)
            continue
        va = parse_arg(a)
        r = read(va)
        if r is None:
            print(f'{a}: 0x{va:08X} is not inside any DOL section')
            continue
        line = (f'{a:24s} VA 0x{r["va"]:08X}  file 0x{r["off"]:06X}  '
                f'{r["bytes4"]} -> f32 {r["f32"]!r}')
        if 'f64' in r:
            line += f'   |  {r["bytes8"]} -> f64 {r["f64"]!r}'
        print(line)
    return 0


if __name__ == '__main__':
    sys.exit(main())
