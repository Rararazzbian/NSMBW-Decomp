import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

WORKDIR = os.path.join(ROOT, "wip", "m_pad", "scratch", "batch1")
TARGET = os.path.join(ROOT, "scratch", "gemini_round8", "auto_03_8016F330_text.o.txt")
SRC = os.path.join(WORKDIR, "m_pad.cpp")
OBJ = os.path.join(WORKDIR, "m_pad.o")
TXT = os.path.join(WORKDIR, "m_pad.txt")
INC = os.path.join(WORKDIR, "inc")

def run(fn):
    ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[INC])
    if not ok:
        print("COMPILE FAILED:\n", log[:3000])
        return False
    dok, dlog = harness.disasm(OBJ, TXT)
    if not dok:
        print("DISASM FAILED:\n", dlog[:2000])
        return False
    matched, report = harness.diff_fn(TARGET, TXT, fn)
    print(("MATCH " if matched else "DIFF  ") + fn)
    if not matched:
        print(report)
    return matched

if __name__ == "__main__":
    fn = sys.argv[1] if len(sys.argv) > 1 else "endPad__4mPadFv"
    run(fn)
