import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_switch2 import jump_tables

HERE=os.path.dirname(os.path.abspath(__file__))
fns=corpus.all_functions()

def switch_blocks(body):
    """(ncases_at_this_level, has_default, case_texts) for each switch, nesting-aware."""
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
        # walk blk, tracking depth; keep 'case' only at depth 1
        depth=0; cases=[]; hasdef=False; p=0
        while p < len(blk):
            c=blk[p]
            if c=='{': depth+=1
            elif c=='}': depth-=1
            elif depth==1 and blk.startswith('case', p) and (p==0 or not blk[p-1].isalnum()):
                e=blk.find(':', p)
                if e>0: cases.append(blk[p+4:e].strip())
                p=e if e>0 else p
            elif depth==1 and blk.startswith('default', p):
                hasdef=True
            p+=1
        out.append((len(cases), hasdef, cases, m.start()))
    return out

rows=[]
for f in fns:
    jts=jump_tables(f)
    b=srcmap.find_body(f.src,f.demangled)
    if not b: continue
    sw=switch_blocks(b[3])
    if not sw and not jts: continue
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],"njt":len(jts),
                 "jtbound":[x[1] for x in jts],
                 "sw":[(s[0],s[1],s[2]) for s in sw]})

# functions with exactly ONE switch -- unambiguous pairing
one=[r for r in rows if len(r["sw"])==1]
print("=== functions with exactly ONE switch statement: %d ===" % len(one))
t=collections.Counter()
for r in one: t[(r["sw"][0][0], r["njt"]>0)]+=1
print("  ncases  jumptable?  n")
for k,v in sorted(t.items()):
    print("   %-7d %-11s %d" % (k[0],k[1],v))

print()
print("=== the boundary: single-switch functions with 6..12 cases ===")
for r in sorted(one, key=lambda r:r["sw"][0][0]):
    n=r["sw"][0][0]
    if 6<=n<=12:
        print("   ncases=%-3d jt=%-5s %s:%d %s bound=%s cases=%s" %
              (n, r["njt"]>0, r["unit"], r["line"], r["fn"], r["jtbound"], r["sw"][0][2]))

print()
print("=== functions with a jump table but MORE than one switch (attribution ambiguous) ===")
for r in rows:
    if r["njt"] and len(r["sw"])!=1:
        print("   %s:%d %s njt=%d switches=%s" % (r["unit"],r["line"],r["fn"],r["njt"],[(s[0],s[2]) for s in r["sw"]]))
json.dump(rows, open(os.path.join(HERE,"sw3_rows.json"),"w"), indent=0)
