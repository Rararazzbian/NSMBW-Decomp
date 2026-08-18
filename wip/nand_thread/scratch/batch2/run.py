import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

WORK = os.path.join(ROOT, "wip", "nand_thread", "scratch", "batch2")
SRC = os.path.join(WORK, "d_nand_thread.cpp")
OBJ = os.path.join(WORK, "draft.o")
TXT = os.path.join(WORK, "draft.txt")
SHADOW = os.path.join(WORK, "shadow")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")

FNS = [
    "cmdExistCheck__13dNandThread_cFv",
    "existCheck__13dNandThread_cFv",
    "cmdSpaceCheck__13dNandThread_cFv",
    "spaceCheck__13dNandThread_cFv",
]

SIZES = {
    "cmdExistCheck__13dNandThread_cFv": 0x70,
    "existCheck__13dNandThread_cFv": 0xD8,
    "cmdSpaceCheck__13dNandThread_cFv": 0x6C,
    "spaceCheck__13dNandThread_cFv": 0x94,
}

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[SHADOW])
if not ok:
    print("COMPILE FAILED")
    print(log)
    sys.exit(1)
print("compile OK")

dok, dlog = harness.disasm(OBJ, TXT)
if not dok:
    print("DISASM FAILED")
    print(dlog)
    sys.exit(1)
print("disasm OK")

for name in FNS:
    want = harness.extract(TARGET, name)
    got = harness.extract(TXT, name)
    print("=" * 70)
    if want is None:
        print(name, "-> TARGET MISSING")
        continue
    if got is None:
        print(name, "-> DRAFT MISSING")
        continue
    want_bytes = len(want) * 4
    exp_size = SIZES[name]
    size_note = "OK" if want_bytes == exp_size else "MISMATCH vs symbol map size 0x%X" % exp_size
    print(name, "target instr*4 = 0x%X (%s)" % (want_bytes, size_note))
    if want == got:
        print(name, "-> MATCH (%d instructions)" % len(want))
    else:
        print(name, "-> DIFF  size: target %d, draft %d" % (len(want), len(got)))
        n = 0
        for i in range(max(len(want), len(got))):
            a = want[i] if i < len(want) else "<none>"
            b = got[i] if i < len(got) else "<none>"
            if a != b:
                print("  %3d | want: %-44s got: %s" % (i, a, b))
                n += 1
            if n > 400:
                print("  ... truncated")
                break
