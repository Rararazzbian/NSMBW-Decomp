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

import collections
import glob
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import verify_anon as V  # noqa: E402

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


def duplicate_bodied(draft):
    """Names of draft functions that have at least one byte-identical sibling.

    The order gate pairs target to draft by instruction CONTENT, so when several
    functions share a body the pairing is ambiguous and "did the matches come out
    ascending?" can report an artefact of tie-breaking rather than a real defect.
    And the complaint is void either way: swapping byte-identical functions emits
    identical `.text`.

    d_a_peach_castle_sequence.cpp reported an order violation for MULTIPLE ROUNDS
    on exactly this -- four groups of identical bodies, one of them seven-way --
    and then landed cleanly at 44/44.

    CALIBRATION, and it limits what this function can tell you: that same landed
    unit STILL flags seven functions with unique bodies, one of them a `global`
    constructor. So "unique body" does NOT mean "real defect", and neither does
    symbol binding. The ascending test is global -- ONE mis-pairing anywhere
    shifts every later index and flags a cascade of innocent functions.

    Treat a unique body as "not explained by ties", never as "confirmed real".
    The only authority is `progress.py --verify-bin`.
    """
    groups = collections.defaultdict(list)
    for name, instructions in V.functions(draft):
        groups[tuple(V.norm(instructions))].append(name)
    return {n for g in groups.values() if len(g) > 1 for n in g}


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
            shared = duplicate_bodied(draft)
            real = [(a, n) for a, n in late if n not in shared]
            wrong.append((unit, label, late, real))
        else:
            ok.append((unit, label))

    print("FUNCTION ORDER GATE -- the linker places .text in definition order,")
    print("so a unit failing this cannot link at ANY match count.\n")

    for unit, label, late, real in wrong:
        if not real:
            print("  order?       %-24s %-8s %d flagged, ALL have byte-identical"
                  % (unit, label, len(late)))
            print("               siblings -- tie artefact, NOT a defect. Swapping")
            print("               identical functions emits identical .text.")
        else:
            print("  order?       %-24s %-8s %d flagged, %d not explained by ties"
                  % (unit, label, len(late), len(real)))
            for addr, name in real[:4]:
                print("               %s  %s" % (addr, name))
    for unit, label in ok:
        print("  ok           %-24s %s" % (unit, label))

    print("")
    unexplained = sum(1 for _, _, _, real in wrong if real)
    print("%d flagged with unexplained entries, %d tie-artefact only, %d ok, "
          "%d NOT CHECKED, %d landed."
          % (unexplained, len(wrong) - unexplained, len(ok), len(skipped), len(landed)))
    if wrong:
        print("")
        print("A flag is NOT proof of a defect. The landed d_a_peach_castle_sequence.cpp")
        print("still flags seven functions here, one of them a global constructor, and")
        print("it links and verifies green. The ascending test is global, so one")
        print("mis-pairing flags a cascade. If a unit is N/N on content and only order")
        print("objects, LAND IT AND SEE -- progress.py --verify-bin is the authority.")
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
