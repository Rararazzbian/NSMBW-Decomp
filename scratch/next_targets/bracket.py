"""For a candidate .text VA range, show the bracketing landed slices in EVERY
section, plus every symbol that falls in the corresponding unclaimed hole.

Assumption stated explicitly: the linker lays TUs down in the SAME relative order
in every section.  So the landed TU whose .text ends just below the candidate
also owns the .rodata/.data/.bss/... immediately below it, and likewise above.
"""
import sys, collections
sys.path.insert(0, r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\next_targets")
from symmap import load_syms, by_sec, load_slices

SECTIONS = [".text", ".ctors", ".dtors", ".rodata", ".data", ".bss",
            ".sdata", ".sbss", ".sdata2", ".sbss2"]

def main():
    lo = int(sys.argv[1], 16); hi = int(sys.argv[2], 16)
    syms = load_syms(); bs = by_sec(syms)
    sl, sec, claimed = load_slices()
    tb = sec[".text"][0]
    # order landed slices by their .text start
    order = []
    for s in sl["slices"]:
        mr = s.get("memoryRanges", {})
        if ".text" not in mr:
            continue
        a = int(mr[".text"].split("-")[0], 16)
        order.append((a, s["source"], mr))
    order.sort()
    lo_off, hi_off = lo - tb, hi - tb
    before = [o for o in order if o[0] < lo_off]
    after = [o for o in order if o[0] >= hi_off]
    print("candidate .text VA 0x%08X-0x%08X (off 0x%X-0x%X, size 0x%X)" % (lo, hi, lo_off, hi_off, hi - lo))
    print()
    for secname in SECTIONS:
        if secname not in sec:
            continue
        base = sec[secname][0]
        pv = None
        for _, src, mr in before:
            if secname in mr:
                pv = (src, int(mr[secname].split("-")[1], 16))
        nx = None
        for _, src, mr in after:
            if secname in mr:
                nx = (src, int(mr[secname].split("-")[0], 16))
                break
        if pv is None and nx is None:
            continue
        p_end = pv[1] if pv else 0
        n_start = nx[1] if nx else None
        holevas = (base + p_end, base + n_start if n_start is not None else None)
        insy = [s for s in bs.get(secname, [])
                if s.addr >= holevas[0] and (holevas[1] is None or s.addr < holevas[1])]
        print("--- %s  base 0x%08X" % (secname, base))
        print("    prev landed: %-45s ends   off 0x%-8X VA 0x%08X" % (pv[0] if pv else "-", p_end, base + p_end))
        print("    next landed: %-45s starts off 0x%-8X VA 0x%08X" % (
            nx[0] if nx else "-", n_start if n_start is not None else 0,
            base + n_start if n_start is not None else 0))
        print("    hole size 0x%X   %d symbols" % (
            ((n_start - p_end) if n_start is not None else 0), len(insy)))
        for s in insy:
            print("        0x%08X off 0x%-8X sz 0x%-6X %-9s %s" % (
                s.addr, s.addr - base, s.size, s.scope or "-", s.name[:85]))
        print()

if __name__ == "__main__":
    main()
