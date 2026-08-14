"""Check a draft object's SECTION SIZES against the slice ranges it claims.

Why this exists
---------------
`verify_anon.py` checks `.text` only, and a unit can be 10/10 in `.text` while
being structurally wrong. `d_a_wm_grid.cpp` was: every authored function was
byte-identical, but the class declared 23 virtuals where the original has 22,
so `__vt__` was 0x64 against a claimed 0x60. An extra trailing vtable slot
changes no function's code at all -- it only makes the vtable object four bytes
too long -- so a `.text` check cannot see it. Landing it broke `d_basesNP.rel`
AND `d_profileNP.rel`, because the surplus shifted `g_profile_WM_GRID`, which
another module's pointer table references.

Reading the result
------------------
`.text` OVER is normal and expected. Unreferenced weak symbols (inline ctors,
dtors, and any header-defined member the TU never calls) are emitted into the
object but never placed by the linker, so the object is routinely larger than
the span the slice claims.

`.data`, `.rodata` and `.bss` OVER is a REAL defect. Those hold vtables,
profile objects, string literals and statics -- all of them referenced, all of
them placed. A surplus there shifts every symbol after it.

UNDER in any section means something the original has is missing entirely.

For a `.data`/`.rodata` mismatch the symbol table for that section is printed,
which is what localised grid's defect in one step.

Usage
-----
    python wip/wm_units/check_sections.py <draft.cpp|draft.o> <module> \
        '{".text": "0x164210-0x164404", ".data": "0x44c90-0x44d20", ...}'

    python wip/wm_units/check_sections.py <draft.o> d_basesNP --dump

`--dump` skips the claim and just prints every section with its symbols, which
is the right first move on a unit whose bounds are not settled yet.
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'tools', 'auto_decomp'))
import harness as H  # noqa: E402

# Sections the linker places wholesale, where a surplus really does shift
# everything downstream. `.text` is deliberately absent -- see the docstring.
STRICT = ('.data', '.rodata', '.bss', '.sdata', '.sdata2', '.sbss', '.ctors', '.dtors')


def _cstr(blob, off):
    end = blob.index(b'\0', off)
    return blob[off:end].decode('utf-8', 'replace')


def read_elf(path):
    """Return {name: {'size', 'index'}} and [(section, value, size, name)] for symbols.

    A small big-endian ELF32 reader rather than a readelf dependency: this has
    to run on Windows next to the rest of the toolchain, and the object layout
    is the only thing being asked about.
    """
    blob = open(path, 'rb').read()
    if blob[:4] != b'\x7fELF':
        raise SystemExit('%s is not an ELF object' % path)
    shoff, = struct.unpack_from('>I', blob, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('>HHH', blob, 0x2E)

    raw = []
    for i in range(shnum):
        base = shoff + i * shentsize
        name, _typ, _flags, _addr, off, size, link, _info, _align, entsize = \
            struct.unpack_from('>10I', blob, base)
        raw.append(dict(name=name, off=off, size=size, link=link, entsize=entsize))

    strtab = raw[shstrndx]['off']
    for s in raw:
        s['sname'] = _cstr(blob, strtab + s['name'])

    sections = {}
    for i, s in enumerate(raw):
        if s['sname']:
            sections[s['sname']] = dict(size=s['size'], index=i)

    symbols = []
    for s in raw:
        if s['sname'] != '.symtab':
            continue
        names = raw[s['link']]['off']
        for off in range(s['off'], s['off'] + s['size'], s['entsize'] or 16):
            n, value, size, info, _other, shndx = struct.unpack_from('>IIIBBH', blob, off)
            nm = _cstr(blob, names + n)
            if nm and shndx < len(raw):
                symbols.append((raw[shndx]['sname'], value, size, nm, (info >> 4) == 2))
    return sections, symbols


def strong_extent(symbols, section, align=4):
    """End offset of the last NON-WEAK symbol in a section.

    Weak symbols are placed only if no other TU already provides them, so a
    weak surplus is not a defect. `d_a_wm_tower.cpp` emits a weak
    `__vt__13dWmObjActor_c` (0x78) that landed `d_a_wm_cloud.cpp` also emits --
    exactly one of them gets placed, and the object is 0x78 "over" either way.
    Strong symbols (GLOBAL and LOCAL) are always placed, so their extent is the
    floor on what this unit must own.
    """
    end = 0
    for sec, value, size, _name, weak in symbols:
        if sec == section and not weak:
            end = max(end, value + size)
    return (end + align - 1) & ~(align - 1)


def dump_symbols(symbols, section):
    rows = sorted(x for x in symbols if x[0] == section and x[2])
    if not rows:
        print('      (no sized symbols in %s)' % section)
        return
    print('      %-8s %-8s %-6s %s' % ('offset', 'size', 'bind', 'symbol'))
    for _sec, value, size, name, weak in rows:
        print('      %#08x %#8x %-6s %s' % (value, size, 'WEAK' if weak else 'strong', name))


def main():
    argv = sys.argv[1:]
    if len(argv) < 2:
        print(__doc__)
        return 1
    src, module = argv[0], argv[1]
    claim = {}
    if len(argv) > 2 and argv[2] != '--dump':
        claim = json.loads(argv[2])

    obj = src
    if src.endswith('.cpp'):
        obj = os.path.splitext(src)[0] + '.sectioncheck.o'
        ok, log = H.compile_draft(src, obj, module=module)
        if not ok:
            print('compile failed:\n' + log)
            return 2

    sections, symbols = read_elf(obj)

    if not claim:
        print('%-12s %8s' % ('section', 'size'))
        for name in sorted(sections):
            if sections[name]['size']:
                print('%-12s %8s' % (name, hex(sections[name]['size'])))
        for name in STRICT + ('.text',):
            if sections.get(name, {}).get('size'):
                print('\n  %s:' % name)
                dump_symbols(symbols, name)
        return 0

    print('%-10s %8s %8s  %s' % ('section', 'claim', 'object', 'verdict'))
    bad = 0
    for name, rng in claim.items():
        lo, hi = (int(x, 16) for x in rng.split('-'))
        want = hi - lo
        got = sections.get(name, {}).get('size', 0)
        # The strong-symbol extent is only meaningful for the STRICT sections.
        # In `.text` weak functions are freely interleaved *between* strong
        # ones, so max(value+size) over strong symbols sweeps up all the weak
        # gaps below the last one and wildly overstates what must be placed --
        # it failed the landed, 5/5-verified d_a_wm_grid.cpp when applied there.
        strong = strong_extent(symbols, name) if name in STRICT else 0
        if got == want:
            verdict = 'ok'
        elif want < strong:
            verdict = 'UNDER %#x of strong symbols -- REAL DEFECT' % (strong - want)
            bad += 1
        elif got < want:
            verdict = 'UNDER %#x  -- something is missing' % (want - got)
            bad += 1
        elif name in STRICT:
            verdict = 'over %#x, all weak (%#x strong) -- ok' % (got - want, strong)
        else:
            verdict = 'over %#x  (weak symbols, expected)' % (got - want)
        print('%-10s %8s %8s  %s' % (name, hex(want), hex(got), verdict))

    for name, rng in claim.items():
        lo, hi = (int(x, 16) for x in rng.split('-'))
        got = sections.get(name, {}).get('size', 0)
        if got != hi - lo and name in STRICT and (hi - lo) < strong_extent(symbols, name):
            print('\n  %s symbols:' % name)
            dump_symbols(symbols, name)

    print('\n%s' % ('SECTIONS CLEAN' if not bad else '%d section(s) wrong -- NOT ready to land' % bad))
    return 0 if not bad else 1


if __name__ == '__main__':
    raise SystemExit(main())
