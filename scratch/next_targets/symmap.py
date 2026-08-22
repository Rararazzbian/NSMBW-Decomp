import re, json, collections, struct, os

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"

LINE = re.compile(r"^(\S+)\s*=\s*([.\w]+):0x([0-9A-Fa-f]+);\s*//\s*(.*)$")

class Sym:
    __slots__ = ("name", "sec", "addr", "size", "type", "scope", "attrs")
    def __init__(s, name, sec, addr, attrs):
        s.name, s.sec, s.addr, s.attrs = name, sec, addr, attrs
        m = re.search(r"size:0x([0-9A-Fa-f]+)", attrs)
        s.size = int(m.group(1), 16) if m else 0
        m = re.search(r"type:(\w+)", attrs)
        s.type = m.group(1) if m else "?"
        m = re.search(r"scope:(\w+)", attrs)
        s.scope = m.group(1) if m else ""
    def __repr__(s):
        return "%s %s:0x%08X size=0x%X %s" % (s.name, s.sec, s.addr, s.size, s.scope)

def load_syms(path=None):
    path = path or (ROOT + r"\bin\dtk\wiimj2d_symbols.txt")
    out = []
    for ln in open(path, encoding="utf-8", errors="replace"):
        m = LINE.match(ln.strip())
        if m:
            out.append(Sym(m.group(1), m.group(2), int(m.group(3), 16), m.group(4)))
    out.sort(key=lambda s: (s.sec, s.addr))
    return out

def by_sec(syms):
    d = collections.defaultdict(list)
    for s in syms:
        d[s.sec].append(s)
    for k in d:
        d[k].sort(key=lambda s: s.addr)
    return d

def load_slices(path=None):
    path = path or (ROOT + r"\slices\wiimj2d.json")
    sl = json.load(open(path))
    sec = {k: (int(v["addr"], 16), int(v["size"], 16)) for k, v in sl["meta"]["sections"].items()}
    claimed = collections.defaultdict(list)
    for s in sl["slices"]:
        nm = s.get("source", "?")
        for k, rng in s.get("memoryRanges", {}).items():
            a, b = rng.split("-")
            claimed[k].append((int(a, 16), int(b, 16), nm))
    for k in claimed:
        claimed[k].sort()
    return sl, sec, claimed

def read_ctors():
    """Return list of (ctors_va, target_va) from original/wiimj2d.dol."""
    dol = open(ROOT + r"\original\wiimj2d.dol", "rb").read()
    # parse DOL header
    text_off = struct.unpack(">7I", dol[0:28])
    data_off = struct.unpack(">11I", dol[0x1C:0x1C+44])
    text_addr = struct.unpack(">7I", dol[0x48:0x48+28])
    data_addr = struct.unpack(">11I", dol[0x64:0x64+44])
    text_size = struct.unpack(">7I", dol[0x90:0x90+28])
    data_size = struct.unpack(">11I", dol[0xAC:0xAC+44])
    segs = []
    for o, a, s in zip(text_off, text_addr, text_size):
        if s: segs.append((a, a+s, o))
    for o, a, s in zip(data_off, data_addr, data_size):
        if s: segs.append((a, a+s, o))
    def va2off(va):
        for a, b, o in segs:
            if a <= va < b:
                return o + (va - a)
        return None
    sl, sec, claimed = load_slices()
    base, size = sec[".ctors"]
    off0 = int(sl["meta"]["sections"][".ctors"].get("offset", "0x0"), 16)
    start = base + off0
    out = []
    for i in range(size // 4):
        va = start + i * 4
        fo = va2off(va)
        (tgt,) = struct.unpack(">I", dol[fo:fo+4])
        out.append((va, tgt))
    return out

if __name__ == "__main__":
    syms = load_syms()
    print("symbols:", len(syms))
    c = read_ctors()
    print("ctors entries:", len(c))
