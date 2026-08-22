import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_loops import analyse
from sweep_loops5 import entry_jump
FOR = re.compile(r'\bfor\s*\(([^;]*);([^;]*);([^)]*)\)')
fns=corpus.all_functions()
rows=[]
for f in fns:
    ls=analyse(f)
    if len(ls)!=1: continue
    b=srcmap.find_body(f.src,f.demangled)
    if not b: continue
    body=b[3]
    fs=list(FOR.finditer(body))
    if len(fs)!=1 or re.search(r'\b(while|do)\b',body): continue
    m=fs[0]; l=ls[0]
    cond=m.group(2).strip(); init=m.group(1).strip()
    rhs=re.split(r'<=|>=|<|>|!=|==',cond)[-1].strip()
    startzero = bool(re.search(r'=\s*0\s*$', init))
    if re.fullmatch(r'-?[0-9]+|0x[0-9A-Fa-f]+',rhs): bf="literal"
    elif re.fullmatch(r'[A-Z_][A-Z0-9_]*',rhs) or re.match(r'(smc_|sc_)',rhs): bf="named-const"
    else: bf="runtime"
    ej=entry_jump(f,l["head"],l["tail"])
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],"bf":bf,"cond":cond,
                 "startzero":startzero,"ej":ej is not None,"kind":l["kind"]})
print("=== `for` loops: is the loop entered by an unconditional jump to the test? ===")
print("  (a jump means MWCC could NOT prove the first iteration runs)")
t=collections.Counter((r["bf"],r["startzero"],r["ej"]) for r in rows)
for k,v in sorted(t.items()):
    print("   bound=%-11s init-to-0=%-6s entry-jump=%-6s n=%d" % (k[0],k[1],k[2],v))
print()
print("=== EXCEPTIONS: constant bound from 0 but STILL has an entry jump ===")
n=0
for r in rows:
    if r["bf"] in ("literal","named-const") and r["startzero"] and r["ej"]:
        n+=1
        if n<=20: print("   %s:%d %s cond=%r" % (r["unit"],r["line"],r["fn"],r["cond"]))
print("   total",n)
print()
print("=== EXCEPTIONS: runtime bound but NO entry jump ===")
n=0
for r in rows:
    if r["bf"]=="runtime" and r["ej"]==False and r["kind"]!="BDNZ":
        n+=1
        if n<=25: print("   %s:%d %s cond=%r kind=%s" % (r["unit"],r["line"],r["fn"],r["cond"],r["kind"]))
print("   total",n)
