import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

def run(name, extra_inc=()):
    base = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_e", "probes")
    src = os.path.join(base, name + ".cpp")
    obj = os.path.join(base, name + ".o")
    txt = os.path.join(base, name + ".txt")
    ok, log = harness.compile_draft(src, obj, extra_inc=extra_inc)
    if not ok:
        print("=== %s: COMPILE FAILED ===" % name)
        print(log)
        return
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print("=== %s: DISASM FAILED ===" % name)
        print(dlog)
        return
    print("=== %s: disasm ===" % name)
    with open(txt, encoding="utf-8", errors="replace") as fh:
        print(fh.read())

if __name__ == "__main__":
    name = sys.argv[1] if len(sys.argv) > 1 else "p1"
    run(name)
