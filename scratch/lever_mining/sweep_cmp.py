import sys, os, re, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus

fns = corpus.all_functions()

# Classify every float compare by the branch that consumes it.
BR = {"beq","bne","blt","ble","bgt","bge","bso","bns",
      "beqlr","bnelr","bltlr","blelr","bgtlr","bgelr","b","bdnz"}

stats = collections.Counter()
sites = []          # (unit, fn, kind, detail)

for f in fns:
    ins = f.insns
    for i,(a,mn,ops,t) in enumerate(ins):
        if mn not in ("fcmpo","fcmpu"): continue
        # walk forward up to 6 instructions for cror / branch
        cror = None
        br   = None
        for j in range(i+1, min(i+8, len(ins))):
            m2 = ins[j][1]
            if m2 == "cror":
                cror = ins[j]; continue
            if m2 in BR:
                br = ins[j]; break
            if m2 in ("fcmpo","fcmpu"): break
        if br is None:
            stats["NO-BRANCH"] += 1
            continue
        kind = ("CROR" if cror else "PLAIN") + "/" + br[1]
        stats[kind] += 1
        sites.append((f.unit, f.demangled or f.name, kind, cror[2] if cror else "", br[2], a))

print("=== float compare -> branch shapes ===")
for k,v in stats.most_common():
    print("%-16s %5d" % (k,v))
print("total float cmp sites:", sum(stats.values()))

# ---- integer compares ----
istats = collections.Counter()
isites = []
ICMP = ("cmpwi","cmpw","cmplwi","cmplw","cmpi","cmp","cmpl","cmpli")
for f in fns:
    ins=f.insns
    for i,(a,mn,ops,t) in enumerate(ins):
        if mn not in ICMP: continue
        cror=None; br=None
        for j in range(i+1, min(i+8,len(ins))):
            m2=ins[j][1]
            if m2=="cror": cror=ins[j]; continue
            if m2 in BR: br=ins[j]; break
            if m2 in ICMP: break
        if br is None:
            istats["NO-BRANCH"]+=1; continue
        signed = mn in ("cmpwi","cmpw","cmpi","cmp")
        kind=("CROR" if cror else "PLAIN")+"/"+("S" if signed else "U")+"/"+br[1]
        istats[kind]+=1
        isites.append((f.unit, f.demangled or f.name, kind, a))

print()
print("=== integer compare -> branch shapes ===")
for k,v in istats.most_common():
    print("%-20s %5d" % (k,v))
print("total int cmp sites:", sum(istats.values()))

import json
json.dump([{"unit":s[0],"fn":s[1],"kind":s[2],"cror":s[3],"br":s[4],"addr":s[5]} for s in sites],
          open(os.path.join(os.path.dirname(os.path.abspath(__file__)),"fcmp_sites.json"),"w"), indent=0)
json.dump([{"unit":s[0],"fn":s[1],"kind":s[2],"addr":s[3]} for s in isites],
          open(os.path.join(os.path.dirname(os.path.abspath(__file__)),"icmp_sites.json"),"w"), indent=0)
