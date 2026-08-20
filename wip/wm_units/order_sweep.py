"""Sweep every unit for the FUNCTION ORDER gate, across all naming styles.

Why this exists
---------------
THREE units were found unlinkable in a single day -- WM_KILLER, WM_KILLERBULLET
and WM_KOOPAJR -- each showing a respectable match count while its `.text`
definition order disagreed with the target's address order. The linker lays
`.text` down in definition order, so such a unit cannot link at ANY tally.

`check_fn_order.py` catches this only where the target address is embedded in the
function's own name (`unk_1684A0`, `fn_2_16D940`). That is 3 drafts out of 56;
WM_KOOPAJR uses real names and was invisible to it.

`verify_anon.py` catches it for ANY naming, because it pairs target to draft on
instruction CONTENT and then asks whether the matched draft indices ascend. This
runs that check across every unit whose `build.py` records its range and target
objects, and NAMES the units it cannot check rather than passing over them in
silence -- "not checked" must never render as "checked and fine".

Read-only: it disassembles nothing and compiles nothing, so it is safe to run
while agents are working. It reads each unit's existing `draft.txt`, which means
a stale draft gives a stale answer -- rebuild first if the result matters.

Usage
-----
    python order_sweep.py
"""

import glob
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
UNITS = os.path.join(ROOT, "wip", "wm_units")
VERIFY = os.path.join(UNITS, "verify_anon.py")

RANGE = re.compile(r"verify_anon\.py \{\}\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)")


def landed_units():
    """Units already in slices/. Their wip draft.txt is a STALE ARTEFACT.

    The real source lives in source/ and is byte-perfect by definition -- it is
    verified against retail every build. Judging a landed unit by the leftover
    draft in its old scratch directory produces alarming nonsense: the sweep's
    first run reported WM_MANTA as "16/16 and ORDER WRONG", i.e. a fully-matching
    unit that cannot link, for a unit that shipped hours earlier.
    """
    path = os.path.join(ROOT, "slices", "d_basesNP.json")
    with open(path, "r", encoding="utf-8", errors="replace") as handle:
        raw = handle.read()
    return {m.lower() for m in re.findall(r"d_a_wm_(\w+)\.cpp", raw)}
OBJS = re.compile(r"objs\s*=\s*\[(.*?)\]", re.S)


def unit_config(build_py):
    with open(build_py, "r", encoding="utf-8", errors="replace") as handle:
        text = handle.read()
    span = RANGE.search(text)
    objs = OBJS.search(text)
    if not span or not objs:
        return None
    paths = re.findall(r"['\"]([^'\"]+\.o)['\"]", objs.group(1))
    return span.group(1), span.group(2), paths


def main():
    ok, wrong, skipped, landed = [], [], [], []
    already = landed_units()

    for build_py in sorted(glob.glob(os.path.join(UNITS, "agent_*", "build.py"))):
        unit = os.path.basename(os.path.dirname(build_py))
        draft = os.path.join(os.path.dirname(build_py), "draft.txt")

        if unit[len("agent_"):].lower() in already:
            landed.append(unit)
            continue

        config = unit_config(build_py)
        if not config:
            skipped.append((unit, "build.py records no range/objs"))
            continue
        if not os.path.exists(draft):
            skipped.append((unit, "no draft.txt -- never built"))
            continue

        lo, hi, objs = config
        missing = [o for o in objs if not os.path.exists(os.path.join(ROOT, o))]
        if missing:
            skipped.append((unit, "target object missing: %s" % missing[0]))
            continue

        result = subprocess.run(
            [sys.executable, VERIFY, draft, lo, hi] + objs,
            cwd=ROOT, capture_output=True, text=True,
        )
        out = result.stdout
        tally = re.search(r"(\d+)/(\d+) byte-identical", out)
        label = "%s/%s" % tally.groups() if tally else "?"
        if "FUNCTION ORDER IS WRONG" in out:
            late = re.findall(r"^\s*(0x[0-9a-f]+)\s+(\S+)\s+<-- defined too late",
                              out, re.M)
            wrong.append((unit, label, late))
        else:
            ok.append((unit, label))

    print("FUNCTION ORDER GATE -- the linker places .text in definition order,")
    print("so a unit failing this cannot link at ANY match count.\n")

    for unit, label, late in wrong:
        print("  ORDER WRONG  %-24s %-8s %d defined too late" % (unit, label, len(late)))
        for addr, name in late[:6]:
            print("               %s  %s" % (addr, name))
    for unit, label in ok:
        print("  ok           %-24s %s" % (unit, label))

    print("")
    print("%d wrong, %d ok, %d NOT CHECKED, %d already landed."
          % (len(wrong), len(ok), len(skipped), len(landed)))
    if landed:
        print("")
        print("Already landed, so skipped -- their wip draft.txt is a stale")
        print("artefact that says nothing about the shipped source:")
        print("    " + ", ".join(landed))
    if skipped:
        print("\nNOT CHECKED -- these were not examined at all, which is not the")
        print("same as passing. Give each a build.py recording its range and")
        print("target objects, or run verify_anon.py by hand:")
        for unit, why in skipped:
            print("    %-24s %s" % (unit, why))
    return 0


if __name__ == "__main__":
    sys.exit(main())
