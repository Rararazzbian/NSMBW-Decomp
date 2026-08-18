import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BATCH = os.path.join(ROOT, 'wip', 'nand_thread', 'scratch', 'batch4')
SRC = os.path.join(BATCH, 'd_nand_thread.cpp')
OBJ = os.path.join(BATCH, 'd_nand_thread.o')
TXT = os.path.join(BATCH, 'd_nand_thread.txt')
TARGET = os.path.join(ROOT, 'wip', 'nand_thread', 'target_raw.txt')
SHADOW_INC = os.path.join(BATCH, 'shadow_include')

FUNCS = {
    'cmdLoad__13dNandThread_cFv': 0x6C,
    'load__13dNandThread_cFv': 0x284,
    'checkCRC__13dNandThread_cFv': 0xCC,
    'cmdDeleteFile__13dNandThread_cFv': 0x6C,
    'deleteFile__13dNandThread_cFv': 0x6C,
}

def main():
    which = sys.argv[1:] or list(FUNCS.keys())
    ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[SHADOW_INC])
    if not ok:
        print("COMPILE FAILED:\n" + log)
        return 1
    dok, dlog = harness.disasm(OBJ, TXT)
    if not dok:
        print("DISASM FAILED:\n" + dlog)
        return 1
    allok = True
    for fn in which:
        size = FUNCS.get(fn)
        matched, report = harness.diff_fn(TARGET, TXT, fn)
        # extract for size check
        insns = harness.extract(TXT, fn)
        icount = len(insns) if insns else 0
        size_ok = (icount * 4 == size) if size else None
        print("=" * 70)
        print(fn, " size_expected=", hex(size) if size else None,
              " instr*4=", icount * 4, " size_ok=", size_ok)
        print("MATCH" if matched else "DIFF")
        if not matched:
            print(report)
            allok = False
    return 0 if allok else 1

if __name__ == '__main__':
    sys.exit(main())
