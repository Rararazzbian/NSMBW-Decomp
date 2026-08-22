import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_loops import analyse

HERE = os.path.dirname(os.path.abspath(__file__))
fns = corpus.all_functions()

rows=[]
for f in fns:
    ls=analyse(f)
    if not ls: continue
    for l in ls:
        seg=f.insns[l["head"]:l["tail"]+1]
        ncall=sum(1 for a,mn,o,t in seg if mn in ("bl","bctrl","bctr"))
        nmtctr=sum(1 for a,mn,o,t in seg if mn=="mtctr")
        rows.append({"unit":f.unit,"fn":f.demangled,"kind":l["kind"],
                     "guard":l["guard"],"ncall":ncall,"len":l["len"],"nmtctr":nmtctr})

print("=== ALL %d loop back-edges: emitted shape vs presence of a CALL in the loop body ===" % len(rows))
t=collections.Counter()
for r in rows: t[(r["kind"], r["ncall"]>0)]+=1
print("  %-13s %-10s %s" % ("emitted","has-call","n"))
for k,v in sorted(t.items(), key=lambda x:(-x[1])):
    print("  %-13s %-10s %d" % (k[0],k[1],v))

print()
print("=== BDNZ loops that DO contain a call (exceptions) ===")
n=0
for r in rows:
    if r["kind"]=="BDNZ" and r["ncall"]>0:
        n+=1; print("  %s %s ncall=%d" % (r["unit"],r["fn"],r["ncall"]))
print("  total", n)

# among call-free loops, what else predicts bdnz?
cf=[r for r in rows if r["ncall"]==0]
print()
print("=== among the %d CALL-FREE back-edges ===" % len(cf))
t2=collections.Counter(r["kind"] for r in cf)
for k,v in t2.most_common(): print("  %-13s %d" % (k,v))

# length distribution
print()
print("=== loop body length (instructions) by shape, call-free only ===")
for k in ("BDNZ","BOTTOM-TEST","TOP-TEST"):
    L=sorted(r["len"] for r in cf if r["kind"]==k)
    if L: print("  %-12s n=%3d  min=%d med=%d max=%d" % (k,len(L),L[0],L[len(L)//2],L[-1]))
json.dump(rows, open(os.path.join(HERE,"loop_rows.json"),"w"), indent=0)
