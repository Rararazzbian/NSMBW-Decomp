import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap

HERE=os.path.dirname(os.path.abspath(__file__))
fns=corpus.all_functions()

CBR={"beq","bne","blt","ble","bgt","bge"}

def dispatch_regions(f):
    """Find runs of >=3 compares of the SAME register against immediates."""
    ins=f.insns
    out=[]
    i=0
    while i < len(ins):
        a,mn,ops,t = ins[i]
        if mn not in ("cmpwi","cmplwi"):
            i+=1; continue
        parts=[p.strip() for p in ops.split(",")]
        reg = parts[-2] if len(parts)>=2 else None
        if reg is None: i+=1; continue
        run=[(i,parts[-1],None)]
        j=i+1
        # the branch that consumes this compare
        br=None
        while j < len(ins) and j < i+40:
            m2=ins[j][1]
            if m2 in ("cmpwi","cmplwi"):
                p2=[p.strip() for p in ins[j][2].split(",")]
                if p2[-2]==reg:
                    run.append((j,p2[-1],br)); br=None
                    j+=1; continue
                else: break
            if m2 in CBR and br is None:
                br=m2
            if m2 in ("bl","bctrl","blr","bctr"): break
            j+=1
        if len(run)>=3:
            out.append((reg,run,i,j))
            i=j
        else:
            i+=1
    return out

rows=[]
jt=0
for f in fns:
    ins=f.insns
    has_bctr = any(mn=="bctr" for a,mn,o,t in ins)
    regs=dispatch_regions(f)
    if not has_bctr and not regs: continue
    b=srcmap.find_body(f.src,f.demangled)
    if not b: continue
    body=b[3]
    nswitch=len(re.findall(r'\bswitch\s*\(', body))
    ncase=len(re.findall(r'\bcase\b', body))
    # count if/else-if chains of length >= 3
    chains=0
    for m in re.finditer(r'\belse\s+if\b', body): chains+=1
    # shape of the compare run
    shapes=[]
    for reg,run,i,j in regs:
        brs=[r[2] for r in run[1:] if r[2]]
        eq=sum(1 for x in brs if x=="beq"); ne=sum(1 for x in brs if x=="bne")
        shapes.append(("BEQ-LADDER" if eq>ne else ("BNE-CHAIN" if ne>eq else "MIXED"), len(run)))
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],"bctr":has_bctr,
                 "nswitch":nswitch,"ncase":ncase,"elseif":chains,"shapes":shapes})

print("=== functions with a jump table (bctr) or a >=3-way compare run: %d ===" % len(rows))
print()
print("A) JUMP TABLE (bctr) vs source construct")
t=collections.Counter()
for r in rows:
    if not r["bctr"]: continue
    t[(r["nswitch"]>0, r["ncase"]>=4)]+=1
for k,v in sorted(t.items()): print("   src-has-switch=%-6s >=4-cases=%-6s n=%d" % (k[0],k[1],v))
print()
print("   bctr functions with NO `switch` in the source body:")
n=0
for r in rows:
    if r["bctr"] and r["nswitch"]==0:
        n+=1
        if n<=25: print("     %s:%d %s elseif=%d" % (r["unit"],r["line"],r["fn"],r["elseif"]))
print("     total",n)

print()
print("B) compare-run shape vs source construct (non-jump-table functions)")
t=collections.Counter()
for r in rows:
    if r["bctr"]: continue
    for sh,ln in r["shapes"]:
        t[(sh, r["nswitch"]>0, r["elseif"]>=2)]+=1
print("   shape        src-switch  src-elseif-chain  n")
for k,v in sorted(t.items(), key=lambda x:-x[1]):
    print("   %-12s %-11s %-17s %d" % (k[0],k[1],k[2],v))

print()
print("C) BEQ-LADDER runs whose source has NO switch")
n=0
for r in rows:
    if r["bctr"]: continue
    if any(s[0]=="BEQ-LADDER" for s in r["shapes"]) and r["nswitch"]==0:
        n+=1
        if n<=25: print("     %s:%d %s elseif=%d shapes=%s" % (r["unit"],r["line"],r["fn"],r["elseif"],r["shapes"]))
print("     total",n)
print()
print("D) BNE-CHAIN runs whose source HAS a switch")
n=0
for r in rows:
    if r["bctr"]: continue
    if any(s[0]=="BNE-CHAIN" for s in r["shapes"]) and r["nswitch"]>0:
        n+=1
        if n<=25: print("     %s:%d %s nswitch=%d ncase=%d shapes=%s" % (r["unit"],r["line"],r["fn"],r["nswitch"],r["ncase"],r["shapes"]))
print("     total",n)
json.dump(rows, open(os.path.join(HERE,"switch_rows.json"),"w"), indent=0)
