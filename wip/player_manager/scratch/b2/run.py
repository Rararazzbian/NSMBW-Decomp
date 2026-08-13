import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

WORK = os.path.join(ROOT, "wip", "player_manager", "scratch", "b2")
SRC = os.path.join(WORK, "draft.cpp")
OBJ = os.path.join(WORK, "draft.o")
TXT = os.path.join(WORK, "draft.txt")
SHADOW = os.path.join(WORK, "shadow")
TARGET = os.path.join(ROOT, "wip", "player_manager", "target_text.txt")

FNS = [
    ("create__9daPyMng_cFiP7mVec3_ciUc", "create__9daPyMng_cFiP7mVec3_ciUc"),
    ("createCourseInit__9daPyMng_cFv", "createCourseInit__9daPyMng_cFv"),
    ("fn_8005F4D0", "fn_8005f4d0__9daPyMng_cFP7mVec3_cii"),
    ("fn_8005f570__9daPyMng_cF16PLAYER_POWERUP_ei", "fn_8005f570__9daPyMng_cF16PLAYER_POWERUP_ei"),
]

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

for target_name, draft_name in FNS:
    want = harness.extract(TARGET, target_name)
    got = harness.extract(TXT, draft_name)
    print("=" * 70)
    if want is None:
        print(target_name, "-> TARGET MISSING")
        continue
    if got is None:
        print(target_name, "-> DRAFT MISSING (", draft_name, ")")
        continue
    if want == got:
        print(target_name, "-> MATCH (%d instructions)" % len(want))
    else:
        print(target_name, "-> DIFF  size: target %d, draft %d" % (len(want), len(got)))
        n = 0
        for i in range(max(len(want), len(got))):
            a = want[i] if i < len(want) else "<none>"
            b = got[i] if i < len(got) else "<none>"
            if a != b:
                print("  %3d | want: %-44s got: %s" % (i, a, b))
                n += 1
            if n > 40:
                print("  ... truncated")
                break
