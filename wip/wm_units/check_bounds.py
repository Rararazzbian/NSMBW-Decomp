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
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


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

        for esec, elo, ehi, src in existing:
            if esec == sec and lo < ehi and elo < hi:
                print('  OVERLAPS %s (%#x-%#x)' % (src, elo, ehi))
                bad += 1

    print('\n%s' % ('BOUNDS PLAUSIBLE' if not bad else '%d problem(s) -- do not build this' % bad))
    return 0 if not bad else 1


if __name__ == '__main__':
    raise SystemExit(main())
