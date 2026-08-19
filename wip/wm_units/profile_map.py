"""Resolve every `g_profile_*` in a REL to the classInit it actually points at.

Why this exists
---------------
**A unit's `.text` starts AT its classInit, and the only place that address is
written down is the first word of the profile OBJECT -- which is a relocation,
not a value you can read out of the symbol map.**

Deriving a range from a profile's own `.data` address instead has mis-scoped a
unit twice on this project, both times expensively:

  WM_ANTLION      dispatched as 0x15ab40-0x15b450; the real range is
                  0x15AC80-0x15B590. BOTH ends were wrong -- the low end reached
                  back into the previous unit, the high end cut 0x140 off.
  WM_ANTLION_MNG  scoped at ~79 functions; it is 22. The 79 was the combined
                  span running on through WM_BOARD.

Neither error is visible from the symbol map, because a profile object sits in
`.data` near -- but not at a fixed offset from -- the code it names. The
`profile - 0x34` folklore is a heuristic and it does not hold across the family.

How it works
------------
Walks the REL's relocation stream (import table at 0x28, entries `>HBBI` =
(running offset delta, type, section, addend); type 202 restarts the running
offset and names the section being patched, 203 ends a stream, 201 is a no-op).
For each `g_profile_*` symbol in `.data` it looks up the relocation patching
that exact address and reports its addend -- the classInit in `.text`.

Sorted by classInit, consecutive rows give each unit's range directly: a unit
runs from its own classInit to the NEXT one.

Usage
-----
    python wip/wm_units/profile_map.py <module> [lo] [hi]

e.g.
    python wip/wm_units/profile_map.py d_basesNP 0x15e000 0x165000
"""
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

TEXT_SECTION = 1
DATA_SECTION = 5


def relocations(module):
    """{(patched_section, patched_addr): (target_section, addend)} for the whole REL."""
    path = os.path.join(ROOT, 'original', module + '.rel')
    if not os.path.exists(path):
        raise SystemExit('no REL at %s' % path)
    b = open(path, 'rb').read()
    impOff, impSize = struct.unpack_from('>II', b, 0x28)
    out = {}
    for i in range(0, impSize, 8):
        _mid, roff = struct.unpack_from('>II', b, impOff + i)
        pos, addr, dest = roff, 0, None
        while pos + 8 <= len(b):
            o, t, sec, add = struct.unpack_from('>HBBI', b, pos)
            pos += 8
            if t == 203:            # R_DOLPHIN_END
                break
            if t == 202:            # R_DOLPHIN_SECTION -- names the patched section
                dest, addr = sec, 0
                continue
            addr += o
            if t != 201:            # 201 = R_DOLPHIN_NOP
                out.setdefault((dest, addr), (sec, add))
    return out


def profiles(module):
    """[(data_addr, name)] for every `g_profile_*` in `.data`."""
    path = os.path.join(ROOT, 'bin', 'dtk', module + '_symbols.txt')
    pat = re.compile(r'^(g_profile_\S+)\s*=\s*\.data:(0x[0-9A-Fa-f]+)')
    out = []
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = pat.match(line.strip())
            if m:
                out.append((int(m.group(2), 16), m.group(1)))
    return sorted(out)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    module = sys.argv[1]
    lo = int(sys.argv[2], 0) if len(sys.argv) > 2 else 0
    hi = int(sys.argv[3], 0) if len(sys.argv) > 3 else 1 << 32

    rel = relocations(module)
    rows = []
    for addr, name in profiles(module):
        r = rel.get((DATA_SECTION, addr))
        # A profile whose first word is not a .text relocation is not a class
        # profile at all -- report it rather than silently dropping it.
        rows.append((r[1] if r and r[0] == TEXT_SECTION else None, name, addr))

    known = sorted([r for r in rows if r[0] is not None])
    for i, (ci, name, addr) in enumerate(known):
        if not (lo <= ci < hi):
            continue
        end = known[i + 1][0] if i + 1 < len(known) else None
        span = '.text 0x%06x-0x%06x  (0x%x B)' % (ci, end, end - ci) if end else \
               '.text 0x%06x-?' % ci
        print('%-40s profile@.data:0x%05x  %s' % (name, addr, span))

    unresolved = [r for r in rows if r[0] is None]
    if unresolved:
        print('\n%d profile symbol(s) with no .text relocation at their first word:'
              % len(unresolved))
        for _ci, name, addr in unresolved:
            print('    %-40s .data:0x%05x' % (name, addr))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
