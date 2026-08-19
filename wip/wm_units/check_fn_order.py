"""Check a draft's function DEFINITION ORDER against the target's .text order.

Why this exists
---------------
WM_KILLER sat on record at 22/23 for rounds while its own build script printed
"FUNCTION ORDER IS WRONG" every single time. The tally looked like the only
blocker, so nobody read the warning. It was a second, unrelated blocker: the
draft defined `unk_168590` before `unk_1684A0`, and **the linker places `.text`
in definition order**, so the unit could not have linked even at 23/23.

A matched-function COUNT is not a landability measure. Order is a separate gate,
and it is free to check -- the target address is embedded in the function's own
name (`unk_1684A0`, `fn_2_16D940`, `R_2_1_16D940`), so definition order must be
ascending by that address. **This is a text comparison, not a compile**, which
means there is no excuse for finding out late.

Usage
-----
    python check_fn_order.py                      # sweep every unit draft
    python check_fn_order.py path/to/draft.cpp    # one file
"""

import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# A definition, not a declaration or a call: `Type Class::name(args) {`
DEFINITION = re.compile(
    r"^[A-Za-z_][\w:<>,\s\*&]*?\b(\w+)::(\w+)\s*\([^;]*?\)\s*(?:const\s*)?\{",
    re.MULTILINE,
)
# The target address carried in the symbol's own name.
ADDRESS_IN_NAME = re.compile(r"(?:unk_|fn_\d+_|R_\d+_\d+_)([0-9A-Fa-f]{4,8})$")


def definitions(path):
    with open(path, "r", encoding="utf-8", errors="replace") as handle:
        text = handle.read()
    # Strip comments so a commented-out definition or a prose mention cannot match.
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)

    found = []
    for match in DEFINITION.finditer(text):
        name = match.group(2)
        addr = ADDRESS_IN_NAME.search(name)
        if addr:
            line = text[: match.start()].count("\n") + 1
            found.append((int(addr.group(1), 16), name, line))
    return found


def check(path):
    found = definitions(path)
    if not found:
        return "skipped"

    inversions = []
    for i in range(1, len(found)):
        if found[i][0] < found[i - 1][0]:
            inversions.append((found[i - 1], found[i]))

    rel = os.path.relpath(path, ROOT)
    if not inversions:
        print("  OK    %-58s %d addressed definitions, ascending" % (rel, len(found)))
        return False

    print("  ORDER %-58s %d inversion(s)" % (rel, len(inversions)))
    for earlier, later in inversions:
        print("          line %-5d %-28s (0x%X)" % (earlier[2], earlier[1], earlier[0]))
        print("          line %-5d %-28s (0x%X)  <-- must come FIRST"
              % (later[2], later[1], later[0]))
    return True


def main():
    if len(sys.argv) > 1:
        targets = sys.argv[1:]
    else:
        targets = sorted(glob.glob(os.path.join(ROOT, "wip", "wm_units", "*", "*.cpp")))

    print("Function definition order vs target .text order")
    print("(the linker places .text in definition order; names carry the address)")
    print("")

    bad = 0
    skipped = []
    for path in targets:
        base = os.path.basename(path)
        if base.startswith("probe") or "_backup" in base or "test" in base:
            continue
        result = check(path)
        if result == "skipped":
            skipped.append(os.path.relpath(path, ROOT))
        elif result:
            bad += 1

    print("")
    print("%d file(s) with ordering inversions." % bad)

    # Never let a silent skip read as a clean bill of health. A draft whose
    # functions carry REAL names has no address in the symbol, so this check
    # cannot see it at all -- and printing nothing would imply it passed.
    if skipped:
        print("")
        print("%d file(s) NOT CHECKED. Their functions use real names, so no target"
              % len(skipped))
        print("address is recoverable from the source text and this tool CANNOT see")
        print("their order. Use the unit's own build.py -- its verify_anon step")
        print("reports FUNCTION ORDER IS WRONG once the object is compiled.")
        for path in skipped:
            print("    %s" % path)
    return 0



if __name__ == "__main__":
    sys.exit(main())
