"""Scout a candidate unit: what pools a `.text` range actually reaches.

Why this exists
---------------
A slice is per TRANSLATION UNIT, not per profile. Deriving a unit's bounds from
a profile boundary has mis-scoped units on this project twice, both expensively:
WM_ANTLION was dispatched with BOTH ends wrong, and WM_ANTLION_MNG was scoped at
~79 functions when it is 22 (the span ran on through WM_BOARD).

`bin/dtk/dtk_splits_*.txt` does NOT help here -- it is generated from the slices
already landed, so it lists only solved units and says nothing about the ones
still to scope.

What DOES decide it is pool ownership. A TU's `.rodata` and `.data` are laid down
contiguously, so if two adjacent profiles reach into one shared, interleaved pool
span they are one TU; if each reaches a disjoint span, they are two. This walks
the relocation stream for a `.text` range and reports every section it touches,
plus whether it owns a `.ctors` entry (one per TU with static state).

Read the OVERLAP between neighbours, not the ranges in isolation. Interleaving is
the evidence; adjacency alone is not.

Usage
-----
    python scout_unit.py d_basesNP 0x1204e0 0x120510      # one profile
    python scout_unit.py d_basesNP 0x1204e0 0x120f00      # the candidate TU
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from profile_map import relocations, profiles

SECTIONS = {1: ".text", 2: ".ctors", 3: ".dtors", 4: ".rodata", 5: ".data", 6: ".bss"}
TEXT_SECTION = 1
DATA_SECTION = 5


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 1
    module = sys.argv[1]
    lo, hi = int(sys.argv[2], 0), int(sys.argv[3], 0)

    rel = relocations(module)

    units = []
    for data_addr, name in profiles(module):
        entry = rel.get((DATA_SECTION, data_addr))
        if entry and entry[0] == TEXT_SECTION:
            units.append((entry[1], name))
    units.sort()

    print("scouting %s .text 0x%X-0x%X  (0x%X bytes)" % (module, lo, hi, hi - lo))
    covered = [name for start, name in units if lo <= start < hi]
    print("profiles whose classInit falls inside: %s"
          % (", ".join(covered) if covered else "(none)"))
    print("")

    reach = {}
    for (patched_section, patched_addr), (tgt_section, addend) in rel.items():
        if patched_section != TEXT_SECTION or not (lo <= patched_addr < hi):
            continue
        reach.setdefault(tgt_section, []).append(addend)

    for section in sorted(reach):
        values = sorted(set(reach[section]))
        internal = [v for v in values if lo <= v < hi] if section == TEXT_SECTION else []
        external = [v for v in values if not (lo <= v < hi)] if section == TEXT_SECTION else values
        label = SECTIONS.get(section, "sec%d" % section)
        print("%-8s %3d distinct targets   0x%X .. 0x%X"
              % (label, len(values), values[0], values[-1]))
        if section == TEXT_SECTION:
            print("         %d inside this range, %d OUTSIDE it"
                  % (len(internal), len(external)))
            if external:
                print("         outside: %s%s"
                      % (", ".join("0x%X" % v for v in external[:8]),
                         " ..." if len(external) > 8 else ""))
                print("         (calls leaving the range are normal; RELOCATIONS INTO")
                print("          this range FROM outside would mean the claim is short)")

    # .ctors ownership: one entry per TU with static state.
    owned = [(pa, add) for (ps, pa), (ts, add) in rel.items()
             if ps == 2 and ts == TEXT_SECTION and lo <= add < hi]
    print("")
    if owned:
        for ctors_addr, text_addr in sorted(owned):
            print(".ctors   0x%X -> __sinit at .text 0x%X" % (ctors_addr, text_addr))
        if len(owned) > 1:
            print("         %d entries -- MORE THAN ONE TU, or one TU with several"
                  % len(owned))
            print("         static initialisers. Resolve before claiming a boundary.")
    else:
        print(".ctors   none -- this range has no static initialiser")
    return 0


if __name__ == "__main__":
    sys.exit(main())
