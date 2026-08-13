import sys, os
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

SRC = os.path.join(ROOT, "wip", "player_manager", "scratch", "b3", "draft.cpp")
OBJ = os.path.join(ROOT, "wip", "player_manager", "scratch", "b3", "draft.o")
TXT = os.path.join(ROOT, "wip", "player_manager", "scratch", "b3", "draft.txt")
TARGET = os.path.join(ROOT, "wip", "player_manager", "target_text.txt")
SHADOW = os.path.join(ROOT, "wip", "player_manager", "scratch", "b3", "shadow_include")

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[SHADOW])
if not ok:
    print("COMPILE FAILED")
    print(log)
    sys.exit(1)

ok, log = harness.disasm(OBJ, TXT)
if not ok:
    print("DISASM FAILED")
    print(log)
    sys.exit(1)

fns = [
    "update__9daPyMng_cFv",
    "isPlayerPauseEnable__9daPyMng_cFSc",
    "setPlayer__9daPyMng_cFiP7dAcPy_c",
    "getPlayer__9daPyMng_cFi",
    "decideCtrlPlrNo__9daPyMng_cFv",
    "setYoshi__9daPyMng_cFP10daPlBase_c",
    "releaseYoshi__9daPyMng_cFP10daPlBase_c",
]

for fn in fns:
    matched, report = harness.diff_fn(TARGET, TXT, fn)
    print("=" * 10, fn, "MATCH" if matched else "DIFF", "=" * 10)
    print(report)
    print()
