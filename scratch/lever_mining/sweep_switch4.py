import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_switch2 import jump_tables
from sweep_switch3 import switch_blocks

HERE=os.path.dirname(os.path.abspath(__file__))
fns=corpus.all_functions()

def targets_of(body, startpos):
    """count distinct case TARGETS: a 'case X:' that is not immediately followed
       by another 'case'/'default' label is a target."""
    i=body.index('{', startpos)
    d=0; k=i
    while k<len(body):
        if body[k]=='{': d+=1
        elif body[k]=='}':
            d-=1
            if d==0: break
        k+=1
    blk=body[i:k+1]
    # collect label positions at depth 1
    depth=0; labels=[]; p=0
    while p<len(blk):
        c=blk[p]
        if c=='{': depth+=1
        elif c=='}': depth-=1
        elif depth==1 and (blk.startswith('case',p) or blk.startswith('default',p)) and (p==0 or not (blk[p-1].isalnum() or blk[p-1]=='_')):
            e=blk.find(':',p)
            if e>0: labels.append((p,e)); p=e
        p+=1
    tgt=0; nlab=0
    for idx,(s,e) in enumerate(labels):
        nlab+=1
        nxt = labels[idx+1][0] if idx+1<len(labels) else len(blk)-1
        between = blk[e+1:nxt].strip()
        if between: tgt+=1
    return nlab, tgt

rows=[]
for f in fns:
    jts=jump_tables(f)
    b=srcmap.find_body(f.src,f.demangled)
    if not b: continue
    body=b[3]
    sw=switch_blocks(body)
    if len(sw)!=1: continue
    nlab,tgt = targets_of(body, sw[0][3])
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],"jt":len(jts)>0,
                 "nlab":nlab,"ntgt":tgt,"cases":sw[0][2]})

print("=== single-switch functions: distinct case TARGETS vs jump table ===")
t=collections.Counter((r["ntgt"], r["jt"]) for r in rows)
print("  ntargets  jumptable?  n")
for k,v in sorted(t.items()):
    print("   %-9d %-11s %d" % (k[0],k[1],v))
print()
lo=[r for r in rows if not r["jt"]]
hi=[r for r in rows if r["jt"]]
print("  max targets WITHOUT a jump table:", max(r["ntgt"] for r in lo), "  n(no-jt)=",len(lo))
print("  min targets WITH a jump table   :", min(r["ntgt"] for r in hi), "  n(jt)=",len(hi))
print()
print("=== the overlap zone ===")
for r in sorted(rows,key=lambda r:r["ntgt"]):
    if 6<=r["ntgt"]<=12:
        print("   ntgt=%-3d nlab=%-3d jt=%-5s %s:%d %s" % (r["ntgt"],r["nlab"],r["jt"],r["unit"],r["line"],r["fn"]))
