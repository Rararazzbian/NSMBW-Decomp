"""Check every unit's `build.py` passes ALL target objects overlapping its range.

Why this exists
---------------
dtk splits some functions -- `__sinit` among them -- into their own
`auto_fn_2_<ADDR>_text.o` object rather than folding them into the surrounding
`auto_00_*` block. **A range can be fully covered by address and still be missing
a function, because the split list has a hole in it.**

The failure mode is silent and it FLATTERS you: the omitted function simply does
not appear in `verify_anon.py`'s target listing, so the denominator is quietly
too small and every percentage looks better than it is. CASTLE_BG reported 12/32
when the truth was 12/33, and only noticed because a symbol it expected at a
known address was absent from the listing.

The convention was already established -- WM_KOOPAJR passes
`auto_fn_2_16E490_text.o` and WM_ANCHOR passes `auto_fn_2_15ABD0_text.o`
alongside their `auto_00_*` objects -- but nothing checked for it.

Usage
-----
    python check_target_objs.py
"""

import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RANGE = re.compile(r"verify_anon\.py \{\}\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)")
NAMED = re.compile(r"auto_(?:00_|fn_2_)([0-9A-Fa-f]+)_text\.o$")


def objects(module):
    """[(start_addr, repo_relative_path)] for every .text split object, sorted."""
    found = []
    pattern = os.path.join(ROOT, "bin", "dtkspl", module, "obj", "*_text.o")
    for path in glob.glob(pattern):
        match = NAMED.match(os.path.basename(path))
        if match:
            found.append((int(match.group(1), 16), os.path.relpath(path, ROOT).replace("\\", "/")))
    return sorted(found)


def main():
    module = sys.argv[1] if len(sys.argv) > 1 else "d_basesNP"
    split = objects(module)
    if not split:
        print("no split objects found for %s" % module)
        return 1

    print("Target-object coverage: every auto_00_* AND auto_fn_2_* object")
    print("overlapping a unit's range must be passed to verify_anon.py.\n")

    incomplete = 0
    checked = 0
    for build_py in sorted(glob.glob(os.path.join(ROOT, "wip", "wm_units", "agent_*", "build.py"))):
        unit = os.path.basename(os.path.dirname(build_py))
        with open(build_py, "r", encoding="utf-8", errors="replace") as handle:
            text = handle.read()
        span = RANGE.search(text)
        if not span:
            continue
        checked += 1
        lo, hi = int(span.group(1), 16), int(span.group(2), 16)
        listed = set(re.findall(r"bin/dtkspl/[^'\"]*\.o", text))

        needed = set()
        for index, (start, path) in enumerate(split):
            end = split[index + 1][0] if index + 1 < len(split) else 1 << 32
            if start < hi and end > lo:
                needed.add(path)

        missing = needed - listed
        if missing:
            incomplete += 1
            print("  INCOMPLETE  %-24s range 0x%x-0x%x" % (unit, lo, hi))
            for path in sorted(missing):
                print("              missing %s" % os.path.basename(path))
                print("              -- its functions are INVISIBLE to the tally,")
                print("                 so the denominator is too small")

    print("")
    print("%d unit(s) checked, %d incomplete." % (checked, incomplete))
    if not incomplete:
        print("Every checked unit passes all overlapping target objects.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
