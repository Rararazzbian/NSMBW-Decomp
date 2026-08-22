import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_loops import analyse

fns=corpus.all_functions()
FOR=re.compile(r'\bfor\s*\(([^;]*);([^;]*);([^)]*)\)')

def ctr_seed(f, head):
    """the immediate loaded into CTR before the loop, if it is a literal."""
    ins=f.insns
    for j in range(head-1, max(-1,head-14), -1):
        if ins[j][1]=="mtctr":
            reg=ins[j][2].strip()
            for k in range(j-1, max(-1,j-8), -1):
                if ins[k][1]=="li" and ins[k][2].split(",")[0].strip()==reg:
                    return int(ins[k][2].split(",")[1].strip(), 0)
                if ins[k][1] in ("lwz","addi","srwi","rlwinm","subi","lbz","lha","lhz","extsb"):
                    p=ins[k][2].split(",")[0].strip()
                    if p==reg: return None
            return None
    return None

rows=[]
for f in fns:
    for l in analyse(f):
        if l["kind"]!="BDNZ": continue
        b=srcmap.find_body(f.src,f.demangled)
        if not b: continue
        seed=ctr_seed(f,l["head"])
        m=FOR.search(b[3])
        cond=m.group(2).strip() if m else ""
        rhs=re.split(r'<=|>=|<|>|!=|==',cond)[-1].strip() if cond else ""
        lit=None
        try: lit=int(rhs,0)
        except Exception: pass
        seg=f.insns[l["head"]:l["tail"]+1]
        earlyexit=any(mn in ("blr","beqlr","bnelr","bltlr","bgtlr","blelr","bgelr") for a,mn,o,t in seg) \
                  or any(mn=="b" for a,mn,o,t in seg)
        # a `return`/`break` inside the source loop body
        rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],"seed":seed,
                     "srclit":lit,"cond":cond,"len":l["len"],"earlyexit":earlyexit})

print("=== BDNZ loops with a LITERAL source bound and a literal CTR seed ===")
print("  src-bound  ctr-seed  ratio  early-exit  function")
t=collections.Counter()
for r in rows:
    if r["srclit"] is None or r["seed"] is None: continue
    ratio = r["srclit"]/float(r["seed"]) if r["seed"] else 0
    t[(round(ratio,2), r["earlyexit"])]+=1
    print("   %-10d %-9d %-6.2f %-11s %s:%d %s" % (r["srclit"],r["seed"],ratio,r["earlyexit"],r["unit"],r["line"],r["fn"]))
print()
print("  ratio x early-exit summary:")
for k,v in sorted(t.items()): print("   ratio=%-5s early-exit=%-6s n=%d" % (k[0],k[1],v))
