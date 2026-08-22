import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap

HERE=os.path.dirname(os.path.abspath(__file__))
fns=corpus.all_functions()

def jump_tables(f):
    """Real jump tables only: cmplwi bound-check + indexed load feeding mtctr."""
    ins=f.insns; out=[]
    for i,(a,mn,o,t) in enumerate(ins):
        if mn!="bctr": continue
        ctx=[ins[j][1] for j in range(max(0,i-8),i)]
        if "lwzx" in ctx or "lwzux" in ctx:
            bound=None
            for j in range(max(0,i-8), i):
                if ins[j][1]=="cmplwi":
                    bound=ins[j][2].split(",")[-1].strip()
            out.append((a,bound))
    return out

def switches_in(body):
    """Return [(n_case_labels, has_default, min_case, max_case, dense?)] per switch."""
    out=[]
    for m in re.finditer(r'\bswitch\s*\(', body):
        i=body.index('{', m.end())
        d=0; k=i
        while k<len(body):
            if body[k]=='{': d+=1
            elif body[k]=='}':
                d-=1
                if d==0: break
            k+=1
        blk=body[i:k+1]
        # only top-level cases of THIS switch: strip nested switch blocks
        cases=re.findall(r'\bcase\s+([^:]+):', blk)
        out.append((len(cases), 'default' in blk, cases))
    return out

rows=[]
for f in fns:
    jts=jump_tables(f)
    b=srcmap.find_body(f.src,f.demangled)
    if not b: continue
    sw=switches_in(b[3])
    if not jts and not sw: continue
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],
                 "njt":len(jts),"jt":jts,
                 "switches":[(c[0],c[1]) for c in sw],"cases":[c[2] for c in sw]})

print("=== REAL jump tables in the corpus ===")
tot=sum(r["njt"] for r in rows)
print("  functions with >=1 jump table:", sum(1 for r in rows if r["njt"]))
print("  jump tables:", tot)
print()
for r in rows:
    if r["njt"]:
        print("  %s:%d %s  jt=%s  switches=%s" % (r["unit"],r["line"],r["fn"],r["jt"],r["switches"]))

print()
print("=== switches in the corpus, by number of case labels, and whether a jump table was emitted ===")
t=collections.Counter()
for r in rows:
    for n,hd in r["switches"]:
        t[(n, r["njt"]>0)]+=1
print("  ncases  jumptable  n")
for k,v in sorted(t.items()):
    print("   %-7d %-10s %d" % (k[0],k[1],v))

print()
print("=== switches with >=6 cases that got NO jump table (exceptions to a density rule) ===")
for r in rows:
    if r["njt"]==0:
        for i,(n,hd) in enumerate(r["switches"]):
            if n>=6:
                print("   %s:%d %s ncases=%d cases=%s" % (r["unit"],r["line"],r["fn"],n,r["cases"][i][:12]))
json.dump(rows, open(os.path.join(HERE,"jt_rows.json"),"w"), indent=0)
