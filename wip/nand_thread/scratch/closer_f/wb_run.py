import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'wb_try.cpp')
OBJ = os.path.join(HERE, 'wb_try.o')
TXT = os.path.join(HERE, 'wb_try.txt')
TARGET = os.path.join(ROOT, 'wip', 'nand_thread', 'target_raw.txt')
NAME = "writeBanner__13dNandThread_cFP12NANDFileInfo"

def main():
    ok, log = harness.compile_draft(SRC, OBJ)
    if not ok:
        print("COMPILE FAILED:")
        print(log)
        return 1
    ok, log = harness.disasm(OBJ, TXT)
    if not ok:
        print("DISASM FAILED:")
        print(log)
        return 1
    matched, report = harness.diff_fn(TARGET, TXT, NAME)
    got = harness.extract(TXT, NAME)
    print("instr count:", len(got), "bytes:", len(got)*4)
    print("MATCH" if matched else "NO MATCH")
    print(report)
    return 0

if __name__ == '__main__':
    sys.exit(main())
