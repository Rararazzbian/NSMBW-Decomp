import sys, os
sys.path.insert(0, r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp")
import harness

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
SRC = os.path.join(ROOT, "wip", "player_manager", "scratch", "b4", "draft.cpp")
OBJ = os.path.join(ROOT, "wip", "player_manager", "scratch", "b4", "draft.o")
TXT = os.path.join(ROOT, "wip", "player_manager", "scratch", "b4", "draft.txt")
TARGET = os.path.join(ROOT, "wip", "player_manager", "target_text.txt")

FUNCS = [
    "getYoshi__9daPyMng_cFi",
    "getYoshiNum__9daPyMng_cFv",
    "getYoshiDirectP__9daPyMng_cFi",
    "getCtrlPlayer__9daPyMng_cFi",
    "getCourseInPlayerModelType__9daPyMng_cFUc",
    "setCarryOverYoshiInfo__9daPyMng_cFUcUci",
    "getYoshiColor__9daPyMng_cFUc",
    "getYoshiFruit__9daPyMng_cFUc",
    "getActScrollInfo__9daPyMng_cFv",
    "getScrollNum__9daPyMng_cFv",
]

ok, out = harness.compile_draft(SRC, OBJ)
if not ok:
    print("COMPILE FAILED")
    print(out)
    sys.exit(1)

ok, out = harness.disasm(OBJ, TXT)
if not ok:
    print("DISASM FAILED")
    print(out)
    sys.exit(1)

for fn in FUNCS:
    matched, msg = harness.diff_fn(TARGET, TXT, fn)
    print("=" * 20, fn, "=" * 20)
    print("MATCH" if matched else "MISMATCH")
    print(msg)
    print()
