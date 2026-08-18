import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

BASE = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_e")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")
FN = "spaceCheck__13dNandThread_cFv"
SHADOW = os.path.join(BASE, "shadow_include")


def run(name, src_path, extra_inc=()):
    obj = os.path.join(BASE, "probes", name + ".o")
    txt = os.path.join(BASE, "probes", name + ".txt")
    ok, log = harness.compile_draft(src_path, obj, extra_inc=extra_inc)
    if not ok:
        print("[%s] COMPILE FAILED" % name)
        print(log[:800])
        return
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print("[%s] DISASM FAILED: %s" % (name, dlog[:300]))
        return
    matched, report = harness.diff_fn(TARGET, txt, FN)
    print("[%s] %s" % (name, "MATCH" if matched else "DIFF"))
    if not matched:
        print(report)
    print()


if __name__ == "__main__":
    src = os.path.join(BASE, "probes", "spacecheck_base.cpp")
    run("sc_baseline", src, extra_inc=[SHADOW])
