import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "d_nand_thread.cpp")
OBJ = os.path.join(HERE, "d_nand_thread.o")
TXT = os.path.join(HERE, "d_nand_thread.txt")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")
SHADOW = os.path.join(HERE, "shadow_include")

FUNCS = [
    "save__13dNandThread_cFv",
    "createBanner__13dNandThread_cFv",
    "writeBanner__13dNandThread_cFP12NANDFileInfo",
]

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[SHADOW])
if not ok:
    print("COMPILE FAILED:")
    print(log)
    sys.exit(1)

dok, dlog = harness.disasm(OBJ, TXT)
if not dok:
    print("DISASM FAILED:")
    print(dlog)
    sys.exit(1)

for fn in FUNCS:
    matched, report = harness.diff_fn(TARGET, TXT, fn)
    print("=" * 78)
    print(fn, "->", "MATCH" if matched else "DIFF")
    if not matched:
        print(report)

# fn_800CF170 (cmdSave) has no name in the target -- compare by address vs our
# real mangled name manually.
want = harness.extract(TARGET, "fn_800CF170")
got = harness.extract(TXT, "cmdSave__13dNandThread_cFPCv") or harness.extract(TXT, "cmdSave")
print("=" * 78)
if got is None:
    print("cmdSave -> DRAFT MISSING (tried cmdSave__13dNandThread_cFPCv)")
elif want == got:
    print("cmdSave (fn_800CF170) -> MATCH (%d instructions)" % len(want))
else:
    print("cmdSave (fn_800CF170) -> DIFF")
    for i in range(max(len(want), len(got))):
        a = want[i] if i < len(want) else "<none>"
        b = got[i] if i < len(got) else "<none>"
        if a != b:
            print("  %3d | want: %-44s got: %s" % (i, a, b))
