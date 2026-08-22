"""Carve an unclaimed .text gap into candidate TUs using __sinit anchors and
class-prefix transitions.  Read-only; prints a report."""
import sys, re, collections
sys.path.insert(0, r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\next_targets")
from symmap import load_syms, by_sec, load_slices, read_ctors

CFRONT = re.compile(r"__(\d+)([A-Za-z_]\w*)")

def cls_of(name):
    """Recover the class name from a CFront-mangled member symbol."""
    m = re.search(r"__(\d+)(\w+)", name)
    if not m:
        return None
    n = int(m.group(1))
    rest = name[m.end(1):]
    if len(rest) >= n:
        return rest[:n]
    return None

def main():
    lo = int(sys.argv[1], 16)
    hi = int(sys.argv[2], 16)
    syms = load_syms()
    bs = by_sec(syms)
    text = [s for s in bs[".text"] if lo <= s.addr < hi]
    ctors = read_ctors()
    ct = {t: v for v, t in ctors}
    print("### gap 0x%08X-0x%08X  size 0x%X  (%d text symbols)" % (lo, hi, hi - lo, len(text)))
    print()
    # sinit anchors
    sinits = [s for s in text if s.name.startswith("__sinit")]
    print("__sinit anchors (%d):" % len(sinits))
    for s in sinits:
        print("   0x%08X size 0x%-6X %s   ctors@0x%08X" % (s.addr, s.size, s.name, ct.get(s.addr, 0)))
    print()
    # class transitions
    prev = None
    run = []
    print("class-prefix runs:")
    for s in text:
        c = cls_of(s.name) or ("<free>" if not s.name.startswith("fn_") else "<fn_>")
        if c != prev:
            if run:
                print("   0x%08X-0x%08X  0x%-6X  %-40s  n=%d  first=%s" % (
                    run[0].addr, s.addr, s.addr - run[0].addr, prev, len(run), run[0].name[:60]))
            run = []
            prev = c
        run.append(s)
    if run:
        print("   0x%08X-0x%08X  0x%-6X  %-40s  n=%d  first=%s" % (
            run[0].addr, hi, hi - run[0].addr, prev, len(run), run[0].name[:60]))

if __name__ == "__main__":
    main()
