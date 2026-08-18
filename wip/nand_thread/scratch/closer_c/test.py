import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

SRC = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_c", "d_nand_thread.cpp")
OBJ = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_c", "d_nand_thread.o")
TXT = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_c", "d_nand_thread.txt")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")

CT = "__ct__13dNandThread_cFiPQ23EGG4Heap"
DT = "__dt__13dNandThread_cFv"

SHADOW = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_c", "shadow_include")

def main():
    ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[SHADOW])
    if not ok:
        print("COMPILE FAILED:")
        print(log)
        return
    dok, dlog = harness.disasm(OBJ, TXT)
    if not dok:
        print("DISASM FAILED:")
        print(dlog)
        return
    for name in (CT, DT):
        matched, report = harness.diff_fn(TARGET, TXT, name)
        print("=== %s: %s ===" % (name, "MATCH" if matched else "DIFF"))
        print(report)
        print()

    # also print emitted function order/sizes for structural check
    print("=== emitted function order ===")
    for n, s in harness.list_functions(TXT, with_size=True):
        print("  %-45s size=0x%x" % (n, s))

if __name__ == "__main__":
    main()
