import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

WORK = os.path.join(ROOT, "wip", "nand_thread", "scratch", "batch2", "realhdr_test")
SRC = os.path.join(WORK, "d_nand_thread.cpp")
OBJ = os.path.join(WORK, "t.o")
TXT = os.path.join(WORK, "t.txt")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")

# void version, against the REAL (unmodified) header
src = """#include <game/bases/d_nand_thread.hpp>

void dNandThread_c::cmdExistCheck() {
    if (OSTryLockMutex(&mMutex.mOSMutex)) {
        mError = 0;
        mFileExists = false;
        mState = 1;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
    }
}
"""
with open(SRC, "w") as f:
    f.write(src)

ok, log = harness.compile_draft(SRC, OBJ)  # NOTE: no extra_inc -> real header
print("compile ok:", ok)
if not ok:
    print(log)
    sys.exit(1)
dok, dlog = harness.disasm(OBJ, TXT)
print("disasm ok:", dok)
want = harness.extract(TARGET, "cmdExistCheck__13dNandThread_cFv")
got = harness.extract(TXT, "cmdExistCheck__13dNandThread_cFv")
print("target instr:", len(want), "draft instr:", len(got))
print("match:", want == got)
