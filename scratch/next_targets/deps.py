"""Classify a candidate TU's outbound `bl` targets: internal / landed / unlanded /
unnamed-needing-syms.txt.  DOL only."""
import sys, os, re, subprocess
sys.path.insert(0, r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\next_targets")
from symmap import load_syms, by_sec, load_slices

PROJ = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = PROJ + r"\bin\dtk-windows-x86_64.exe"
TMP = r"C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\ddfa20f8-4e0e-450f-b1f4-c32f19a6e891\scratchpad"

def disasm(objname):
    out = os.path.join(TMP, objname + ".txt")
    if not os.path.exists(out):
        subprocess.run([DTK, "elf", "disasm",
                        os.path.join(PROJ, "bin", "dtkspl", "obj", objname), out],
                       check=True, capture_output=True)
    return open(out, encoding="utf-8", errors="replace").read()

def main():
    lo, hi = int(sys.argv[1], 16), int(sys.argv[2], 16)
    objs = sys.argv[3:]
    txt = "".join(disasm(o) for o in objs)
    tgts = set(re.findall(r"\bbl (\S+)", txt))
    tgts = {t.split("+")[0].strip('"') for t in tgts}
    syms = load_syms(); bs = by_sec(syms)
    byname = {}
    for s in syms:
        byname.setdefault(s.name, s)
    sl, sec, claimed = load_slices()
    tb = sec[".text"][0]
    def owner(va):
        off = va - tb
        for a, b, nm in claimed[".text"]:
            if a <= off < b:
                return nm
        return None
    buckets = {"internal": [], "landed": [], "UNLANDED": [], "unknown": []}
    for t in sorted(tgts):
        s = byname.get(t)
        if s is None:
            m = re.match(r"^fn_([0-9A-Fa-f]{8})$", t)
            if m:
                va = int(m.group(1), 16)
                if lo <= va < hi:
                    buckets["internal"].append(t); continue
                o = owner(va)
                (buckets["landed"] if o else buckets["UNLANDED"]).append(
                    "%s  (%s)" % (t, o or "UNLANDED"))
                continue
            buckets["unknown"].append(t); continue
        if s.sec != ".text":
            buckets["unknown"].append(t); continue
        if lo <= s.addr < hi:
            buckets["internal"].append(t); continue
        o = owner(s.addr)
        if o:
            buckets["landed"].append(t)
        else:
            buckets["UNLANDED"].append("%s  @0x%08X" % (t, s.addr))
    for k in ("UNLANDED", "unknown", "internal", "landed"):
        print("--- %s (%d)" % (k, len(buckets[k])))
        if k in ("UNLANDED", "unknown"):
            for v in buckets[k]:
                print("    ", v)
    print()

if __name__ == "__main__":
    main()
