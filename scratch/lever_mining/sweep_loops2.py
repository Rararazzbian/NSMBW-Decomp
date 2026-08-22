import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_loops import analyse

HERE = os.path.dirname(os.path.abspath(__file__))
fns = corpus.all_functions()

FOR = re.compile(r'\bfor\s*\(([^;]*);([^;]*);([^)]*)\)')

def iv_name(init, inc):
    m=re.search(r'([A-Za-z_][A-Za-z0-9_]*)\s*=', init)
    if m: return m.group(1)
    m=re.search(r'([A-Za-z_][A-Za-z0-9_]*)\s*(\+\+|--)|(\+\+|--)\s*([A-Za-z_][A-Za-z0-9_]*)', inc)
    if m: return m.group(1) or m.group(4)
    return None

def body_of_for(body, m):
    """text between the for(...) closing paren and its matching brace/statement."""
    i=m.end()
    while i<len(body) and body[i] in ' \t\r\n': i+=1
    if i<len(body) and body[i]=='{':
        d=0; k=i
        while k<len(body):
            if body[k]=='{': d+=1
            elif body[k]=='}':
                d-=1
                if d==0: break
            k+=1
        return body[i:k+1]
    j=body.find(';', i)
    return body[i:j+1] if j>0 else body[i:]

rows=[]
for f in fns:
    ls=analyse(f)
    if len(ls)!=1: continue
    b=srcmap.find_body(f.src, f.demangled)
    if not b: continue
    body=b[3]
    fors=list(FOR.finditer(body))
    if len(fors)!=1: continue
    if re.search(r'\b(while|do)\b', body): continue
    m=fors[0]
    iv=iv_name(m.group(1), m.group(3))
    if not iv: continue
    lb=body_of_for(body,m)
    # uses of iv inside the loop body
    uses=[mm for mm in re.finditer(r'\b%s\b'%re.escape(iv), lb)]
    # classify each use: subscript  arr[iv]  (possibly arr[iv].f) vs anything else
    nonsub=0; sub=0
    for mm in uses:
        pre=lb[:mm.start()].rstrip()
        post=lb[mm.end():].lstrip()
        if pre.endswith('[') and post.startswith(']'): sub+=1
        else: nonsub+=1
    # stride
    inc=m.group(3)
    unit = bool(re.search(r'(\+\+|--)', inc)) and not re.search(r'[+\-*/]=', inc)
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1],"kind":ls[0]["kind"],
                 "guard":ls[0]["guard"],"iv":iv,"nsub":sub,"nonsub":nonsub,
                 "unitstride":unit,"cond":m.group(2).strip(),"inc":inc.strip()})

print("=== single plain `for` loops with a resolvable induction variable: %d ===" % len(rows))
t=collections.Counter()
for r in rows:
    t[(r["kind"], r["nonsub"]==0, r["unitstride"])]+=1
print("  emitted        iv-only-as-subscript  unit-stride   n")
for k,v in sorted(t.items(), key=lambda x:-x[1]):
    print("  %-13s %-21s %-13s %d" % (k[0],k[1],k[2],v))

print()
print("=== BDNZ cases where the iv IS used outside a subscript (exceptions) ===")
for r in rows:
    if r["kind"]=="BDNZ" and r["nonsub"]>0:
        print("  %s:%d %s  iv=%s nonsub=%d cond=%r inc=%r" % (r["unit"],r["line"],r["fn"],r["iv"],r["nonsub"],r["cond"],r["inc"]))

print()
print("=== BOTTOM-TEST cases where the iv is ONLY a subscript (exceptions) ===")
n=0
for r in rows:
    if r["kind"]=="BOTTOM-TEST" and r["nonsub"]==0:
        n+=1
        if n<=25:
            print("  %s:%d %s  iv=%s nsub=%d cond=%r inc=%r" % (r["unit"],r["line"],r["fn"],r["iv"],r["nsub"],r["cond"],r["inc"]))
print("  (total %d)" % n)

json.dump(rows, open(os.path.join(HERE,"for_rows.json"),"w"), indent=0)
