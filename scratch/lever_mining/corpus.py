"""Shared parser for the matched-corpus disassembly."""
import json, os, re

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
IDX  = os.path.join(ROOT, "scratch", "lever_mining", "index.json")

INSN_RE = re.compile(r"^/\* ([0-9A-F]{8}) ([0-9A-F]{8})  ([0-9A-F ]{11}) \*/\t(.*)$")
FN_RE   = re.compile(r"^\.fn (\S+?),\s*(\S+)$")
ENDFN_RE= re.compile(r"^\.endfn ")
COMMENT_FN = re.compile(r"^# (.*)$")
LBL_RE  = re.compile(r"^(\S+):$")

class Fn:
    __slots__ = ("name","demangled","unit","src","insns","labels","section")
    def __init__(self, name, demangled, unit, src):
        self.name=name; self.demangled=demangled; self.unit=unit; self.src=src
        self.insns=[]      # list of (addr:int, mnemonic, operands_str, rawtext)
        self.labels={}     # label -> index into insns
    def __repr__(self):
        return "<Fn %s in %s>" % (self.demangled or self.name, self.unit)
    def text(self):
        return "\n".join(i[3] for i in self.insns)

def load_units():
    return json.load(open(IDX))

def parse_disasm(path, unit, src):
    fns=[]
    cur=None
    last_comment=None
    pending_labels=[]
    section=None
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line=line.rstrip("\n")
            s=line.strip()
            if s.startswith(".text") or s.startswith(".data") or s.startswith(".rodata") \
               or s.startswith(".sdata") or s.startswith(".bss") or s.startswith(".sbss"):
                if re.match(r"^\.(text|data|rodata|sdata2?|sbss2?|bss|ctors|dtors)$", s):
                    section=s
                    continue
            m=FN_RE.match(s)
            if m:
                cur=Fn(m.group(1), last_comment, unit, src)
                cur.section=section
                pending_labels=[]
                fns.append(cur)
                continue
            if ENDFN_RE.match(s):
                cur=None; continue
            if s.startswith("#"):
                c=s[1:].strip()
                if c and not c.startswith(".text:") and not c.startswith("0x"):
                    last_comment=c
                continue
            if cur is None: continue
            m=INSN_RE.match(line)
            if m:
                addr=int(m.group(1),16)
                txt=m.group(4).strip()
                # strip trailing /* comment */
                txt=re.sub(r"\s*/\*.*?\*/\s*$","",txt)
                parts=txt.split(None,1)
                mn=parts[0]; ops=parts[1] if len(parts)>1 else ""
                for L in pending_labels:
                    cur.labels[L]=len(cur.insns)
                pending_labels=[]
                cur.insns.append((addr,mn,ops,txt))
                continue
            m=LBL_RE.match(s)
            if m and (not s.startswith(".") or s.startswith(".L")):
                pending_labels.append(m.group(1))
    return fns

_cache=None
def all_functions():
    global _cache
    if _cache is not None: return _cache
    out=[]
    for e in load_units():
        out.extend(parse_disasm(e["disasm"], e["rel"], e["src"]))
    _cache=out
    return out

def read_src(path):
    return open(path,"r",encoding="utf-8",errors="replace").read().split("\n")
