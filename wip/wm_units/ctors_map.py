"""Map every `.ctors` entry in a REL to the unit that owns its static initialiser.

Why this exists
---------------
`.ctors` is a table of pointers to static-initialiser functions, one word per
entry, each word a relocation into `.text`. Resolving those targets against the
profile ranges from `profile_map.py` answers, exactly and cheaply:

  * **does this unit have a static initialiser at all, and how many?**
  * **which `.text` function IS the unit's `__sinit`?**

Both questions come up constantly and are otherwise guessed at. Three agents
independently identified a function as "probably this unit's `__sinit`" on the
same day -- WM_KILLERBULLET's `fn_2_169FA0`, WM_KOOPAJR's `fn_2_16E490`,
WM_HANACHAN's `fn_2_165B20` -- and this table confirmed all three in one run.

The count matters as much as the identity. A unit whose target has exactly ONE
`.ctors` entry but whose draft emits TWO has an extra static initialiser, which
is what a spurious `#include <game/bases/d_wm_lib.hpp>` produces: that header
declares `sc_ForceList` as a `static` array with an `mVec3_c(...)` initialiser,
so every including TU gets its own dynamically-initialised copy and pays a
`.ctors` entry for it. **Comparing the counts is a one-look test that replaces a
round of source-order experiments.**

Usage
-----
    python ctors_map.py d_basesNP              # every entry
    python ctors_map.py d_basesNP WM_KINOPIO   # just units matching a pattern
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from profile_map import relocations, profiles

CTORS_SECTION = 2
TEXT_SECTION = 1
DATA_SECTION = 5


def unit_ranges(module, rel):
    """[(text_start, profile_name)] sorted, derived from each profile's classInit."""
    units = []
    for data_addr, name in profiles(module):
        entry = rel.get((DATA_SECTION, data_addr))
        if entry and entry[0] == TEXT_SECTION:
            units.append((entry[1], name))
    return sorted(units)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    module = sys.argv[1]
    pattern = re.compile(sys.argv[2], re.I) if len(sys.argv) > 2 else None

    rel = relocations(module)
    units = unit_ranges(module, rel)

    def owner(addr):
        for index, (start, name) in enumerate(units):
            end = units[index + 1][0] if index + 1 < len(units) else 1 << 32
            if start <= addr < end:
                return name
        return "(outside any profile range)"

    entries = sorted((pa, add) for (ps, pa), (ts, add) in rel.items()
                     if ps == CTORS_SECTION and ts == TEXT_SECTION)

    counts = {}
    rows = []
    for ctors_addr, text_addr in entries:
        name = owner(text_addr)
        counts[name] = counts.get(name, 0) + 1
        rows.append((ctors_addr, text_addr, name))

    print("%d .ctors entries in %s" % (len(entries), module))
    print("%-14s %-14s %s" % (".ctors", "__sinit .text", "owning profile"))
    shown = 0
    for ctors_addr, text_addr, name in rows:
        if pattern and not pattern.search(name):
            continue
        print("0x%-12x 0x%-12x %s%s"
              % (ctors_addr, text_addr, name,
                 "   <-- %d entries for this unit" % counts[name]
                 if counts[name] > 1 else ""))
        shown += 1

    if pattern and not shown:
        print("(no .ctors entry matches %r -- that unit has NO static initialiser,"
              % sys.argv[2])
        print(" which is itself the answer: its TU includes no header that costs one)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
