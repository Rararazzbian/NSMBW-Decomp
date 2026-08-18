import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'd_nand_thread.cpp')
OBJ = os.path.join(HERE, 'draft.o')
TXT = os.path.join(HERE, 'draft.txt')
TARGET = os.path.join(ROOT, 'wip', 'nand_thread', 'target_raw.txt')

def main():
    extra_inc = sys.argv[2:] if len(sys.argv) > 2 else []
    ok, log = harness.compile_draft(SRC, OBJ, extra_inc=extra_inc)
    if not ok:
        print("COMPILE FAILED:")
        print(log)
        return 1
    ok, log = harness.disasm(OBJ, TXT)
    if not ok:
        print("DISASM FAILED:")
        print(log)
        return 1
    fn = sys.argv[1] if len(sys.argv) > 1 else None
    if fn:
        matched, report = harness.diff_fn(TARGET, TXT, fn)
        print("MATCH" if matched else "NO MATCH")
        print(report)
    return 0

if __name__ == '__main__':
    sys.exit(main())
