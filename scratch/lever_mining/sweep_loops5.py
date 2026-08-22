import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_loops import analyse

HERE=os.path.dirname(os.path.abspath(__file__))
fns=corpus.all_functions()
KW=re.compile(r'\b(for|while|do)\b')

def entry_jump(f, head, tail):
    """Is the loop entered by an unconditional forward branch landing INSIDE
       [head,tail]?  That is the rotated pre-test shape."""
    ins=f.insns; labels=f.labels
    for j in range(0, head):
        if ins[j][1]!="b": continue
        t=ins[j][2].strip().split(",")[-1].strip()
        if t in labels and head <= labels[t] <= tail:
            return labels[t]-head
    return None

rows=[]
for f in fns:
    ls=analyse(f)
    if len(ls)!=1: continue
    b=srcmap.find_body(f.src, f.demangled)
    if not b: continue
    l=ls[0]
    seg=f.insns[l["head"]:l["tail"]+1]
    ncall=sum(1 for a,mn,o,t in seg if mn in ("bl","bctrl","bctr"))
    ej=entry_jump(f,l["head"],l["tail"])
    kws=KW.findall(b[3])
    if kws.count("do")==1 and len(kws)<=2: sk="do-while"
    elif kws==["for"]: sk="for"
    elif kws==["while"]: sk="while"
    else: continue
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],"src":sk,"kind":l["kind"],
                 "entryjump":ej is not None,"ejoff":ej,"ncall":ncall,"len":l["len"]})

print("=== single-loop functions with an unambiguous source keyword: %d ===" % len(rows))
print()
print("source keyword  x  emitted shape  x  entry-jump present")
t=collections.Counter((r["src"],r["kind"],r["entryjump"]) for r in rows)
for k,v in sorted(t.items(), key=lambda x:(x[0][0],-x[1])):
    print("   src=%-9s %-12s entry-jump=%-6s n=%d" % (k[0],k[1],k[2],v))

print()
print("=== do-while cases (all of them) ===")
for r in rows:
    if r["src"]=="do-while":
        print("   %s:%d %s kind=%s entryjump=%s ncall=%d" % (r["unit"],r["line"],r["fn"],r["kind"],r["entryjump"],r["ncall"]))
print()
print("=== `while` WITHOUT an entry jump (would break the rule) ===")
for r in rows:
    if r["src"]=="while" and not r["entryjump"]:
        print("   %s:%d %s kind=%s ncall=%d" % (r["unit"],r["line"],r["fn"],r["kind"],r["ncall"]))
print()
print("=== `for` WITHOUT an entry jump ===")
n=0
for r in rows:
    if r["src"]=="for" and not r["entryjump"]:
        n+=1
        if n<=20: print("   %s:%d %s kind=%s ncall=%d" % (r["unit"],r["line"],r["fn"],r["kind"],r["ncall"]))
print("   total", n)
json.dump(rows, open(os.path.join(HERE,"loop5_rows.json"),"w"), indent=0)
