import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_loops import analyse
from sweep_loops2 import FOR, iv_name, body_of_for

HERE = os.path.dirname(os.path.abspath(__file__))
fns = corpus.all_functions()

rows=[]
for f in fns:
    ls=analyse(f)
    if len(ls)!=1: continue
    b=srcmap.find_body(f.src, f.demangled)
    if not b: continue
    body=b[3]
    fs=list(FOR.finditer(body))
    if len(fs)!=1: continue
    if re.search(r'\b(while|do)\b', body): continue
    m=fs[0]
    l=ls[0]
    seg=f.insns[l["head"]:l["tail"]+1]
    ncall=sum(1 for a,mn,o,t in seg if mn in ("bl","bctrl","bctr"))
    cond=m.group(2).strip()
    rhs=re.split(r'[<>]=?|!=|==', cond)
    rhs=rhs[-1].strip() if len(rhs)>1 else ""
    # bound classification
    if re.fullmatch(r'[0-9]+|0x[0-9A-Fa-f]+', rhs): bnd="literal"
    elif re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', rhs) and (rhs.isupper() or rhs.startswith('smc_') or rhs.startswith('sc_') or rhs.startswith('MAX') ): bnd="named-const"
    elif re.search(r'(m[A-Z]|->|\.|\[|\()', rhs): bnd="loaded"
    else: bnd="local-var"
    lb=body_of_for(body,m)
    stores=bool(re.search(r'(?<![=!<>+\-*/&|^])=(?!=)', lb)) or bool(re.search(r'(\+\+|--)', lb))
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],"kind":l["kind"],
                 "ncall":ncall,"bound":bnd,"stores":stores,"cond":cond,"rhs":rhs})

print("=== single plain `for` loop functions: %d ===" % len(rows))
print()
print("A) call in the body?")
t=collections.Counter((r["kind"], r["ncall"]>0) for r in rows)
for k,v in sorted(t.items(), key=lambda x:-x[1]): print("   %-12s call=%-6s %d" % (k[0],k[1],v))

cf=[r for r in rows if r["ncall"]==0]
print()
print("B) among the %d CALL-FREE ones: bound form x body-stores x shape" % len(cf))
t=collections.Counter((r["bound"], r["stores"], r["kind"]) for r in cf)
for k,v in sorted(t.items()):
    print("   bound=%-12s stores=%-6s %-12s %d" % (k[0],k[1],k[2],v))

print()
print("C) call-free BOTTOM-TEST detail (the ones that did NOT get bdnz)")
for r in cf:
    if r["kind"]!="BDNZ":
        print("   %s:%d %s cond=%r bound=%s stores=%s" % (r["unit"],r["line"],r["fn"],r["cond"],r["bound"],r["stores"]))
print()
print("D) call-free BDNZ with a LOADED bound and stores in the body (would break rule C)")
for r in cf:
    if r["kind"]=="BDNZ" and r["bound"]=="loaded" and r["stores"]:
        print("   %s:%d %s cond=%r" % (r["unit"],r["line"],r["fn"],r["cond"]))
