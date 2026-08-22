import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap

HERE = os.path.dirname(os.path.abspath(__file__))
fns = corpus.all_functions()

CBR = {"beq","bne","blt","ble","bgt","bge","bso","bns","bdnz","bdz"}
UBR = {"b"}
CMP = ("cmpwi","cmpw","cmplwi","cmplw","cmpi","cmp","cmpl","cmpli","fcmpo","fcmpu",
       "rlwinm.","clrlwi.","and.","or.","addic.","cmplwi","andi.","extsb.","subfic")

def analyse(f):
    ins=f.insns
    labels=f.labels
    loops=[]
    for i,(a,mn,ops,t) in enumerate(ins):
        if mn not in CBR and mn not in UBR: continue
        tgt=ops.strip().split(",")[-1].strip()
        if tgt not in labels: continue
        ti=labels[tgt]
        if ti >= i: continue          # not a back-edge
        # back edge found: head=ti, tail=i
        kind=None
        if mn=="bdnz":
            kind="BDNZ"
        elif mn in CBR:
            kind="BOTTOM-TEST"        # conditional back-edge = compare at the bottom
        else:
            kind="TOP-TEST"           # unconditional back-edge => test is at the top
        # guard: is there a conditional forward branch just above the head that
        # skips past the tail?
        guard=False
        for j in range(max(0,ti-4), ti):
            if ins[j][1] in CBR:
                g=ins[j][2].strip().split(",")[-1].strip()
                if g in labels and labels[g] > i:
                    guard=True
        # what feeds the back-edge
        feeder=None
        if kind=="BOTTOM-TEST":
            for j in range(i-1, max(-1,i-4), -1):
                if ins[j][1] in CMP:
                    feeder=ins[j][1]; break
        loops.append({"kind":kind,"head":ti,"tail":i,"len":i-ti,"guard":guard,
                      "br":mn,"feeder":feeder,"addr":a})
    return loops

rows=[]
kindc=collections.Counter()
for f in fns:
    ls=analyse(f)
    if not ls: continue
    for l in ls: kindc[l["kind"]]+=1
    b=srcmap.find_body(f.src, f.demangled)
    rows.append({"unit":f.unit,"fn":f.demangled,"line":b[1] if b else None,
                 "loops":ls,"body":b[3] if b else None})

print("=== loop back-edge shapes across the corpus ===")
for k,v in kindc.most_common(): print("  %-12s %d" % (k,v))
print("  functions with >=1 loop:", len(rows))
print("  total back-edges:", sum(kindc.values()))

# --- source correlation: functions with exactly ONE loop and a mapped body ---
KW = re.compile(r'\b(for|while|do)\b')
single=[r for r in rows if len(r["loops"])==1 and r["body"]]
tab=collections.Counter()
det=collections.defaultdict(list)
for r in single:
    kws=KW.findall(r["body"])
    # a do-while shows up as 'do' followed by 'while'
    if kws.count("do")==1 and len(kws)<=2: sk="do-while"
    elif kws==["for"]: sk="for"
    elif kws==["while"]: sk="while"
    else: sk="mixed:"+",".join(kws) if kws else "NONE"
    l=r["loops"][0]
    tab[(sk, l["kind"], l["guard"])]+=1
    det[(sk,l["kind"],l["guard"])].append(r)

print()
print("=== single-loop functions: source keyword vs emitted shape (%d) ===" % len(single))
for k,v in sorted(tab.items(), key=lambda x:-x[1]):
    print("  src=%-12s emitted=%-12s guard=%-5s n=%d" % (k[0],k[1],str(k[2]),v))

json.dump({"|".join(map(str,k)):[{"unit":x["unit"],"fn":x["fn"],"line":x["line"]} for x in v]
           for k,v in det.items()}, open(os.path.join(HERE,"loop_detail.json"),"w"), indent=0)

# bound form for 'for' loops
print()
print("=== 'for' loops: emitted shape vs bound form ===")
bt=collections.Counter()
for r in single:
    kws=KW.findall(r["body"])
    if kws!=["for"]: continue
    m=re.search(r'\bfor\s*\(([^;]*);([^;]*);([^)]*)\)', r["body"])
    if not m:
        bt[("unparsed", r["loops"][0]["kind"])]+=1; continue
    cond=m.group(2).strip()
    # constant upper bound?
    if re.search(r'[<>]=?\s*[0-9]+\s*$', cond): bform="literal-bound"
    elif re.search(r'[<>]=?\s*[A-Za-z_][A-Za-z0-9_]*\s*$', cond): bform="var-bound"
    elif re.search(r'[<>]=?\s*.*(\(|\[|\.|->)', cond): bform="expr-bound"
    elif cond=="": bform="no-cond"
    else: bform="other"
    bt[(bform, r["loops"][0]["kind"], r["loops"][0]["guard"])]+=1
for k,v in sorted(bt.items(), key=lambda x:-x[1]):
    print("  %-14s %-12s guard=%-5s n=%d" % (k[0],k[1],str(k[2]) if len(k)>2 else "?",v))
