import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap

fns = corpus.all_functions()
BR = {"beq","bne","blt","ble","bgt","bge","bso","bns",
      "beqlr","bnelr","bltlr","blelr","bgtlr","bgelr"}
ICMP = ("cmpwi","cmpw","cmplwi","cmplw","cmpi","cmp","cmpl","cmpli")

REL = re.compile(r'(?<![<>=!])(>=|<=|<<|>>|->|<|>)(?!=)')

def src_rel_ops(body):
    """relational operators in a source body, ignoring shifts, arrows and templates."""
    ops=[]
    # kill template argument lists heuristically: Name<...> with no spaces around
    # strip only genuine template argument lists: Name<Id, Id*, ns::Id>
    b = re.sub(r'\b[A-Za-z_][A-Za-z0-9_:]*<[A-Za-z0-9_:,*&\s]{0,60}>', ' ', body)
    for m in REL.finditer(b):
        o=m.group(1)
        if o in ('<<','>>','->'): continue
        ops.append(o)
    return ops

rows=[]
for f in fns:
    ins=f.insns
    fsites=[]
    for i,(a,mn,ops,t) in enumerate(ins):
        if mn not in ("fcmpo","fcmpu"): continue
        cror=None; br=None
        for j in range(i+1, min(i+8,len(ins))):
            m2=ins[j][1]
            if m2=="cror": cror=ins[j]; continue
            if m2 in BR: br=ins[j]; break
            if m2 in ("fcmpo","fcmpu"): break
        fsites.append(("CROR" if cror else "PLAIN", br[1] if br else None,
                       cror[2] if cror else ""))
    if not fsites: continue
    b=srcmap.find_body(f.src, f.demangled)
    if not b: continue
    nicmp=sum(1 for a,mn,o,t in ins if mn in ICMP)
    ops=src_rel_ops(b[3])
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],
                 "sites":fsites,"nicmp":nicmp,"ops":ops})

json.dump(rows, open(os.path.join(os.path.dirname(os.path.abspath(__file__)),"fcmp_rows.json"),"w"), indent=0)

# clean 1:1 cases -- exactly one float compare, no integer compares, exactly one rel op
clean=[r for r in rows if len(r["sites"])==1 and r["nicmp"]==0 and len(r["ops"])==1]
tab=collections.Counter()
for r in clean:
    tab[(r["ops"][0], r["sites"][0][0], r["sites"][0][1])]+=1
print("=== CLEAN 1:1 (one fcmpo, zero int cmp, one source relational op) : %d functions ===" % len(clean))
for k,v in sorted(tab.items(), key=lambda x:-x[1]):
    print("  src %-3s -> %-6s %-6s  n=%d" % (k[0],k[1],k[2],v))

# aggregate: does a source body containing >= or <= predict a cror?
agg=collections.Counter()
for r in rows:
    has_ge_le = any(o in (">=","<=") for o in r["ops"])
    has_lt_gt = any(o in ("<",">") for o in r["ops"])
    ncror=sum(1 for s in r["sites"] if s[0]=="CROR")
    agg[(has_ge_le, has_lt_gt, ncror>0)]+=1
print()
print("=== aggregate (all %d fns with a float compare and a mapped body) ===" % len(rows))
print("  src_has(>=|<=), src_has(<|>), emits_cror : count")
for k,v in sorted(agg.items()):
    print("  ",k,v)

# exceptions: cror emitted but no >=/<= in source
exc=[r for r in rows if any(s[0]=="CROR" for s in r["sites"]) and not any(o in (">=","<=") for o in r["ops"])]
print()
print("=== CROR with no >= or <= in the source body: %d ===" % len(exc))
for r in exc[:40]:
    print("  %s:%d %s ops=%s" % (r["unit"],r["line"],r["fn"],r["ops"]))

# reverse exceptions: >=/<= in source, only plain branches, exactly one rel op & one site
exc2=[r for r in rows if len(r["sites"])==1 and len(r["ops"])==1 and r["ops"][0] in (">=","<=")
      and r["sites"][0][0]=="PLAIN" and r["nicmp"]==0]
print()
print("=== single >= / <= in source but PLAIN branch: %d ===" % len(exc2))
for r in exc2[:40]:
    print("  %s:%d %s op=%s br=%s" % (r["unit"],r["line"],r["fn"],r["ops"][0],r["sites"][0][1]))
