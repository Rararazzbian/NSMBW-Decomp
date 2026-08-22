import sys
sys.path.insert(0, r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\next_targets")
from symmap import load_syms, by_sec, load_slices

syms = load_syms(); bs = by_sec(syms)
sl, sec, claimed = load_slices()

def gaplist(secname):
    base, size = sec[secname]
    out = []; prev = 0; pn = "<start>"
    for a, b, nm in claimed.get(secname, []):
        if a > prev:
            out.append((prev, a, pn, nm))
        prev = max(prev, b); pn = nm
    if prev < size:
        out.append((prev, size, pn, "<end>"))
    return out, base

if __name__ == "__main__":
    secname = sys.argv[1] if len(sys.argv) > 1 else ".text"
    minsz = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x100
    gaps, base = gaplist(secname)
    sin = [s for s in bs[".text"] if s.name.startswith("__sinit")]
    pre = "__sinit_" + chr(92)
    for lo, hi, pa, pb in sorted(gaps, key=lambda g: g[1] - g[0]):
        if hi - lo < minsz:
            continue
        va0, va1 = base + lo, base + hi
        names = [s.name.replace(pre, "") for s in sin if va0 <= s.addr < va1]
        tag = "GAME" if (pa.startswith("dol/") or pb.startswith("dol/")) else "lib "
        print("%s 0x%08X-0x%08X off 0x%X-0x%X sz 0x%-7X n=%d %s | %s -> %s" % (
            tag, va0, va1, lo, hi, hi - lo, len(names), ",".join(names)[:90], pa, pb))
