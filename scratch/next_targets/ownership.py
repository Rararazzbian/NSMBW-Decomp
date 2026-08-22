"""DOL ownership test.

The REL playbook proves ownership from the module's relocation stream.  The DOL
is fully linked and has none, but dtk's disassembly of a SPLIT OBJECT still
resolves every cross-object reference to a symbol NAME.  So: disassemble the
candidate's .text split objects, harvest every symbol name mentioned, and mark
each symbol sitting in a bracketed data hole as REFERENCED or not.

A symbol in the hole that nothing in the candidate's own .text references is the
same signal check_bounds.py's ownership check looks for.

Usage: ownership.py <loVA> <hiVA> <objdir-listing-cache>
"""
import sys, os, re, subprocess, collections
sys.path.insert(0, r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\next_targets")
from symmap import load_syms, by_sec, load_slices

PROJ = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = PROJ + r"\bin\dtk-windows-x86_64.exe"
TMP = r"C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\ddfa20f8-4e0e-450f-b1f4-c32f19a6e891\scratchpad"

def objs_for(lo, hi):
    names = [n for n in os.listdir(PROJ + r"\bin\dtkspl\obj") if n.startswith("auto_")
             and n.endswith(".o")]
    got = []
    for n in names:
        p = n.split("_")
        try:
            a = int(p[2], 16)
        except (IndexError, ValueError):
            continue
        if p[1] not in ("03",) or not n.endswith("_text.o"):
            continue
        if lo <= a < hi:
            got.append((a, n))
    got.sort()
    return [n for _, n in got]

def disasm(objname):
    out = os.path.join(TMP, objname + ".txt")
    if not os.path.exists(out):
        subprocess.run([DTK, "elf", "disasm",
                        os.path.join(PROJ, "bin", "dtkspl", "obj", objname), out],
                       check=True, capture_output=True)
    return open(out, encoding="utf-8", errors="replace").read()

def main():
    lo, hi = int(sys.argv[1], 16), int(sys.argv[2], 16)
    objlist = objs_for(lo, hi)
    for extra in sys.argv[3:]:
        objlist.append(extra)
    sys.stderr.write("objects: " + ", ".join(objlist) + "\n")
    txt = "".join(disasm(o) for o in objlist)
    raw = set(re.findall(r"[A-Za-z_@$][\w@$<>,.$]*", txt))
    tokens = set()
    for t in raw:
        tokens.add(t)
        for suf in ("@ha", "@l", "@sda21", "@sda2", "@h"):
            if t.endswith(suf):
                tokens.add(t[:-len(suf)])
        # pool symbols print as NAME_VA -- strip the trailing _hex address
        m = re.match(r"^(@[\w$]+?)_[0-9A-Fa-f]{8}$", t.split("@ha")[0].split("@l")[0])
        if m:
            tokens.add(m.group(1))
    more = set()
    for t in list(tokens):
        m = re.match(r"^(.*?)_[0-9A-Fa-f]{8}$", t)
        if m:
            more.add(m.group(1))
    tokens |= more
    syms = load_syms(); bs = by_sec(syms)
    sl, sec, claimed = load_slices()
    tb = sec[".text"][0]
    order = []
    for s in sl["slices"]:
        mr = s.get("memoryRanges", {})
        if ".text" in mr:
            order.append((int(mr[".text"].split("-")[0], 16), s["source"], mr))
    order.sort()
    lo_off, hi_off = lo - tb, hi - tb
    for secname in [".rodata", ".data", ".bss", ".sdata", ".sbss", ".sdata2", ".sbss2"]:
        if secname not in sec:
            continue
        base = sec[secname][0]
        pv = None; nx = None
        for a, src, mr in order:
            if secname in mr and a < lo_off:
                pv = int(mr[secname].split("-")[1], 16)
        for a, src, mr in order:
            if secname in mr and a >= hi_off:
                nx = int(mr[secname].split("-")[0], 16); break
        if pv is None:
            pv = 0
        if nx is None:
            continue
        ins = [s for s in bs.get(secname, []) if base + pv <= s.addr < base + nx]
        if not ins:
            continue
        print("=== %s hole off 0x%X-0x%X (VA 0x%08X-0x%08X)" % (secname, pv, nx, base+pv, base+nx))
        for s in ins:
            hit = s.name in tokens
            print("   %s 0x%08X off 0x%-7X sz 0x%-6X %s" % (
                "REF " if hit else "  . ", s.addr, s.addr - base, s.size, s.name[:80]))
        print()

if __name__ == "__main__":
    main()
