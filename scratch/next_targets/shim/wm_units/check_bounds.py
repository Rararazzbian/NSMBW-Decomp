"""Validate a proposed slice block against the TARGET symbol map, before building.

Why this exists
---------------
Bounds are the single largest source of wasted effort on this project. Today:

  d_a_wm_grid.cpp    .text wrong at BOTH ends, .data 0x10 too high  -> failed landing
  d_a_wm_tower.cpp   same two errors                                 -> failed landing
  d_a_wm_ghost.cpp   .data 0x44a9c-0x44cb4 vs the real 0x44a68-0x44c80

That last one is the important case: **both spans are 0x218 bytes.** The claim
had swapped the unit's own leading strings for the next unit's, and a size-only
check reports `ok`. `check_sections.py` cannot catch it, because it only ever
compares a length against a compiled object. The only way to catch it is to ask
whether the claimed addresses land on real symbol boundaries in the target.

What it checks
--------------
1. Every claimed range starts exactly on a symbol boundary, and ends exactly
   where a symbol ends (or where the next one begins).
2. No overlap with any range already in the slice file.
3. Gaps to the nearest neighbours, reported per section -- an unexplained gap
   usually means an unidentified unit, not free space.
4. Family rule for `wm` actors: a unit's `.data` opens on the two anonymous
   5-byte `sc_ForceList` strings ("F7C0"/"W7C0"), NOT on `g_profile_*`. If the
   first contained symbol is a profile, the claim is 0x34 too high.
5. **OWNERSHIP.** Every symbol inside a claimed range must actually be
   REFERENCED from inside the unit's own `.text`. A claim can end on a perfectly
   real symbol boundary and still be wrong, because the symbol belongs to the
   NEIGHBOUR.

   That is not hypothetical. `d_a_wm_sandpillar.cpp` claimed
   `.rodata 0x8ef8-0x8fa8` and this tool said PLAUSIBLE, because `0x8fa8` is a
   real boundary -- the end of `lbl_2_rodata_8F98`. But every relocation
   targeting `0x8F98` originates at `0x1794ca`, `0x1794de`, `0x17973a` and
   `0x179742`, all OUTSIDE the unit's `.text` claim `[0x177690, 0x179380)`. The
   array belongs to a different class entirely. The real bound is `0x8f98`, and
   with it the unit reports SECTIONS CLEAN.

   Checks 1-3 are all about ADDRESSES lining up. Only this one asks whether the
   content is the unit's, and it is the check that catches a same-shape,
   same-size, wrong-owner claim.

Usage
-----
    python wip/wm_units/check_bounds.py <module> '<slice JSON>' [source-to-skip]

e.g.
    python wip/wm_units/check_bounds.py d_basesNP \
      '{".text": "0x164230-0x164430", ".data": "0x44c80-0x44d20"}'
"""
import json
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


# REL section indices are fixed by the format.
REL_SECTION_INDEX = {'.text': 1, '.ctors': 2, '.dtors': 3, '.rodata': 4,
                     '.data': 5, '.bss': 6}


def referrers(module, lo, hi):
    """Every `.text` address that relocates against [lo, hi), from the REL itself.

    The relocation stream is the only place ownership is actually written down.
    dtk's symbol map says where things START; it does not say who USES them, and
    a claim that ends on a real boundary can still have swept in a neighbour's
    object. Returns a sorted list of referring addresses, or None if the REL is
    unavailable.
    """
    path = os.path.join(ROOT, 'original', module + '.rel')
    if not os.path.exists(path):
        return None
    b = open(path, 'rb').read()
    impOff, impSize = struct.unpack_from('>II', b, 0x28)
    out = set()
    for i in range(0, impSize, 8):
        _mid, roff = struct.unpack_from('>II', b, impOff + i)
        pos, addr = roff, 0
        while pos + 8 <= len(b):
            o, t, _sec, add = struct.unpack_from('>HBBI', b, pos)
            pos += 8
            if t == 203:      # R_DOLPHIN_END
                break
            if t == 202:      # R_DOLPHIN_SECTION -- restart the running offset
                addr = 0
                continue
            addr += o
            if t != 201 and lo <= add < hi:   # 201 = R_DOLPHIN_NOP
                out.add(addr)
    return sorted(out)


def load_symbols(module):
    """[(section, addr, size, name)] from dtk's symbol map, sorted."""
    path = os.path.join(ROOT, 'bin', 'dtk', module + '_symbols.txt')
    out = []
    pat = re.compile(r'^(\S+)\s*=\s*(\.\w+):(0x[0-9A-Fa-f]+);(?:.*size:(0x[0-9A-Fa-f]+))?')
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = pat.match(line.strip())
            if m:
                out.append((m.group(2), int(m.group(3), 16),
                            int(m.group(4), 16) if m.group(4) else 0, m.group(1)))
    out.sort(key=lambda x: (x[0], x[1]))
    return out


def existing_ranges(module):
    path = os.path.join(ROOT, 'slices', module + '.json')
    d = json.load(open(path, encoding='utf-8'))
    out = []
    for s in d['slices']:
        for sec, rng in s.get('memoryRanges', {}).items():
            lo, hi = (int(x, 16) for x in rng.split('-'))
            out.append((sec, lo, hi, s['source']))
    return out


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 1
    module, claim = sys.argv[1], json.loads(sys.argv[2])
    # Optional third arg: a slice `source` to ignore in the overlap check, so a
    # unit that is already landed can be re-validated without overlapping itself.
    skip = sys.argv[3] if len(sys.argv) > 3 else None
    syms = load_symbols(module)
    existing = [e for e in existing_ranges(module) if skip not in (e[3],)]
    bad = 0

    for sec, rng in claim.items():
        lo, hi = (int(x, 16) for x in rng.split('-'))
        print('\n%s  %#x-%#x  (%#x bytes)' % (sec, lo, hi, hi - lo))
        inside = [s for s in syms if s[0] == sec and lo <= s[1] < hi]
        below = [s for s in syms if s[0] == sec and s[1] < lo]
        above = [s for s in syms if s[0] == sec and s[1] >= hi]

        if not inside:
            print('  no symbols in range -- cannot validate (dtk may not label this section)')
            continue

        first, last = inside[0], inside[-1]
        print('  first: %#08x %-32s (%#x)' % (first[1], first[3], first[2]))
        print('  last : %#08x %-32s (%#x)' % (last[1], last[3], last[2]))

        if first[1] != lo:
            print('  START is %#x past the first symbol in range -- claim begins mid-object'
                  % (first[1] - lo))
            bad += 1
        if below and below[-1][2] and below[-1][1] + below[-1][2] > lo:
            print('  START overlaps %s, which ends at %#x'
                  % (below[-1][3], below[-1][1] + below[-1][2]))
            bad += 1

        end = last[1] + last[2]
        if last[2] and end > hi:
            print('  END cuts %s short: it ends at %#x, past the claim' % (last[3], end))
            bad += 1
        elif above and above[0][1] != hi and last[2]:
            gap = above[0][1] - hi
            if gap > 0:
                print('  gap of %#x before the next symbol %s at %#08x -- verify nothing is missing'
                      % (gap, above[0][3], above[0][1]))

        # wm-family rules. Both of these were real, landing-breaking errors, and
        # neither is visible to a size comparison.
        if sec == '.data' and first[3].startswith('g_profile_'):
            print('  .data begins at a PROFILE symbol. A wm unit opens on its two anonymous')
            print('  sc_ForceList strings, so this claim is very likely 0x34 too high.')
            bad += 1
        elif sec == '.data' and first[2] == 0x24:
            print('  .data begins at a 0x24 object -- that is sc_ForceList. The two anonymous')
            print('  5-byte strings it points at come FIRST and belong to this unit, so the')
            print('  claim is very likely 0x10 too high. (This is grid\'s original error.)')
            bad += 1
        if sec == '.text' and above and above[0][2] == 0x1c:
            print('  the next symbol %s is 0x1c -- array-destructor sized. A wm unit ends AFTER'
                  % above[0][3])
            print('  its OWN array destructor, which sits past its __sinit, not at the __sinit.')
            bad += 1

        # OWNERSHIP, per SYMBOL. A symbol belongs to this unit if ANY reference
        # to it originates inside ANY range the unit claims -- not just `.text`.
        # Strings are routinely referenced only from the unit's own `.data`
        # (sc_ForceList points at its two 5-byte names; animation-name arrays
        # point at model names), and a .text-only test false-flags every one of
        # them: it reported 10 problems on the LANDED, byte-exact
        # d_a_wm_ghost.cpp. A whole-RANGE test is equally useless in the other
        # direction, since shared data and profile objects are legitimately
        # referenced from other units -- ghost has 15 of 25 references coming
        # from outside and is correct.
        #
        # What IS evidence is a single symbol with no reference from anywhere
        # inside the unit at all.
        own = []
        for csec, crng in claim.items():
            clo, chi = (int(x, 16) for x in crng.split('-'))
            own.append((clo, chi))
        for saddr, ssize, sname in ((x[1], x[2], x[3]) for x in inside):
            if not ssize or sname.startswith(('gap_', 'pad_')):
                continue
            refs = referrers(module, saddr, saddr + ssize)
            if refs is None:
                break
            if refs and not any(lo2 <= r < hi2 for r in refs for lo2, hi2 in own):
                print('  %s (%#x, %#x bytes) is NEVER referenced from anywhere'
                      % (sname, saddr, ssize))
                print('  inside this unit -- all %d references come from outside every'
                      % len(refs))
                print('  claimed range, e.g. %s.'
                      % ', '.join('%#x' % r for r in refs[:4]))
                print('  This object belongs to a NEIGHBOUR. The claim reaches too far.')
                bad += 1

        for esec, elo, ehi, src in existing:
            if esec == sec and lo < ehi and elo < hi:
                print('  OVERLAPS %s (%#x-%#x)' % (src, elo, ehi))
                bad += 1

    print('\n%s' % ('BOUNDS PLAUSIBLE' if not bad else '%d problem(s) -- do not build this' % bad))
    return 0 if not bad else 1


if __name__ == '__main__':
    raise SystemExit(main())
