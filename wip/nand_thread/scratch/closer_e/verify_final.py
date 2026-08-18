import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

BASE = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_e")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")

SRC = os.path.join(BASE, "d_nand_thread.cpp")
OBJ = os.path.join(BASE, "d_nand_thread.o")
TXT = os.path.join(BASE, "d_nand_thread.txt")

FNS = [
    "__ct__13dNandThread_cFiPQ23EGG4Heap",
    "__dt__Q23EGG5MutexFv",
    "__dt__6mMutexFv",
    "__dt__13dNandThread_cFv",
]

SYMS = {
    "__ct__13dNandThread_cFiPQ23EGG4Heap": 0x118,
    "__dt__Q23EGG5MutexFv": 0x40,
    "__dt__6mMutexFv": 0x40,
    "__dt__13dNandThread_cFv": 0x64,
}


def main():
    ok, log = harness.compile_draft(SRC, OBJ)  # NO shadow -- real header only
    if not ok:
        print("COMPILE FAILED (real header):")
        print(log)
        return
    dok, dlog = harness.disasm(OBJ, TXT)
    if not dok:
        print("DISASM FAILED:", dlog)
        return
    for name in FNS:
        matched, report = harness.diff_fn(TARGET, TXT, name)
        print("=== %s: %s ===" % (name, "MATCH" if matched else "DIFF"))
        if not matched:
            print(report)
        print()

    print("=== emitted function order ===")
    for n, s in harness.list_functions(TXT, with_size=True):
        expect = SYMS.get(n)
        tag = ""
        if expect is not None:
            tag = "  (expect 0x%x)" % expect
        print("  %-45s size=0x%x%s" % (n, s, tag))


if __name__ == "__main__":
    main()
