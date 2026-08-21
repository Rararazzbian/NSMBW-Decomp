"""Decode a literal-pool constant out of the retail DOL, by address or by symbol.

Two separate agents have now been blocked on "I could not determine the value of
`@54951_8042CB1C` from the disassembly", and one of them papered over it by
brute-forcing constants until the bytes matched -- which always succeeds, because
`lfs f1, "@54951_8042CB1C"@sda21(r0)` assembles to `C0 20 00 00` with the offset
field ZEROED. A wrong constant is byte-identical to the right one. The pattern
can never tell you the value; only the binary can.

    python pool.py 0x8042CB1C            # -> float and double reading
    python pool.py @54951_8042CB1C       # the VA is embedded in the symbol name
    python pool.py 0x8042CB1C 0x8042CB48 # several at once

dtk's pool symbol names carry the address after the underscore, so you can paste
one straight out of a disassembly listing.
"""
import os
import re
import struct
import sys

DOL = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__)))), 'original', 'wiimj2d.dol')


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
