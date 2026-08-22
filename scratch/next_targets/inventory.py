import json, re, sys, collections

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"

sl = json.load(open(ROOT + r"\slices\wiimj2d.json"))
SEC = {k: int(v["addr"], 16) for k, v in sl["meta"]["sections"].items()}
SECSZ = {k: int(v["size"], 16) for k, v in sl["meta"]["sections"].items()}

# claimed ranges per section, in OFFSET space (as stored in slice file)
claimed = collections.defaultdict(list)
for s in sl["slices"]:
    name = s.get("name") or s.get("source") or "?"
    for sec, rng in s.get("memoryRanges", {}).items():
        if isinstance(rng, str):
            a, b = rng.split('-')
            lo, hi = int(a, 16), int(b, 16)
        else:
            lo, hi = [int(x, 16) if isinstance(x, str) else x for x in rng]
        claimed[sec].append((lo, hi, name))

for sec in claimed:
    claimed[sec].sort()

def gaps(sec):
    base = SEC.get(sec)
    size = SECSZ.get(sec)
    out = []
    prev = 0
    prevname = "<section start>"
    for lo, hi, nm in claimed[sec]:
        if lo > prev:
            out.append((prev, lo, prevname, nm))
        prev = max(prev, hi)
        prevname = nm
    if size and prev < size:
        out.append((prev, size, prevname, "<section end>"))
    return out

if __name__ == "__main__":
    sec = sys.argv[1] if len(sys.argv) > 1 else ".text"
    base = SEC[sec]
    g = gaps(sec)
    g.sort(key=lambda t: -(t[1] - t[0]))
    print("section %s base 0x%08X size 0x%X  claimed %d slices" % (sec, base, SECSZ[sec], len(claimed[sec])))
    for lo, hi, a, b in g[:40]:
        print("  0x%-8X 0x%-8X  size 0x%-7X  VA 0x%08X-0x%08X   after %s  before %s" % (
            lo, hi, hi - lo, base + lo, base + hi, a, b))
