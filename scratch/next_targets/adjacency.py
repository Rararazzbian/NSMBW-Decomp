"""AGENT_CONTEXT section 6 check 1: overlap-and-adjacency, in the slice file's
own OFFSET space, against slices/wiimj2d.json."""
import sys, json
sys.path.insert(0, r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\next_targets")
from symmap import load_slices

def main():
    claim = json.loads(sys.argv[1])
    sl, sec, claimed = load_slices()
    bad = 0
    for s, rng in claim.items():
        lo, hi = (int(x, 16) for x in rng.split("-"))
        base = sec[s][0]
        print("%-9s off 0x%X-0x%X  VA 0x%08X-0x%08X   base subtracted 0x%08X" % (
            s, lo, hi, base + lo, base + hi, base))
        ov = [c for c in claimed[s] if c[0] < hi and lo < c[1]]
        if ov:
            bad += 1
            for c in ov:
                print("    OVERLAP with %s (0x%X-0x%X)" % (c[2], c[0], c[1]))
        else:
            print("    overlap: none")
        below = [c for c in claimed[s] if c[1] <= lo]
        above = [c for c in claimed[s] if c[0] >= hi]
        b = max(below, key=lambda c: c[1]) if below else None
        a = min(above, key=lambda c: c[0]) if above else None
        if b:
            print("    below: %s ends 0x%X  ->  %s" % (
                b[2], b[1], "EXACTLY ADJACENT" if b[1] == lo else "gap of 0x%X" % (lo - b[1])))
        if a:
            print("    above: %s starts 0x%X  ->  %s" % (
                a[2], a[0], "EXACTLY ADJACENT" if a[0] == hi else "gap of 0x%X" % (a[0] - hi)))
    print("\n%s" % ("OVERLAP CLEAN" if not bad else "%d OVERLAPS" % bad))

if __name__ == "__main__":
    main()
