import sys, os, re, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import corpus, srcmap
from sweep_switch3 import switch_blocks

fns=corpus.all_functions()

def ladders(f, maxgap=4):
    """Tight dispatch ladders: >=3 compares of the same reg against immediates,
       each followed within `maxgap` by its branch, compares no more than
       maxgap*8 apart."""
    ins=f.insns; out=[]; i=0
    while i<len(ins):
        if ins[i][1] not in ("cmpwi","cmplwi"): i+=1; continue
        p=[x.strip() for x in ins[i][2].split(",")]
        reg=p[-2]
        run=[]; j=i; last=i
        while j<len(ins):
            if ins[j][1] in ("cmpwi","cmplwi"):
                p2=[x.strip() for x in ins[j][2].split(",")]
                if p2[-2]!=reg: break
                br=None
                for k in range(j+1, min(j+1+maxgap, len(ins))):
                    if ins[k][1] in ("beq","bne","blt","ble","bgt","bge"):
                        br=ins[k][1]; break
                if br is None: break
                run.append((p2[-1], br)); last=j; j+=1; continue
            if j-last > 60: break
            if ins[j][1] in ("blr","bctr"): break
            j+=1
        if len(run)>=3:
            eq=sum(1 for x in run if x[1]=="beq"); ne=sum(1 for x in run if x[1]=="bne")
            out.append(("BEQ-LADDER" if ne==0 else ("BNE-CHAIN" if eq==0 else "MIXED"), len(run)))
            i=last+1
        else: i+=1
    return out

t=collections.Counter()
ex_beq=[]; ex_bne=[]
for f in fns:
    ls=ladders(f)
    if not ls: continue
    b=srcmap.find_body(f.src,f.demangled)
    if not b: continue
    if any(mn=="bctr" for a,mn,o,t2 in f.insns): continue
    sw=switch_blocks(b[3])
    hassw=len(sw)>0
    for sh,n in ls:
        t[(sh,hassw)]+=1
        if sh=="BEQ-LADDER" and not hassw: ex_beq.append((f.unit,b[1],f.demangled,n))
        if sh=="BNE-CHAIN" and hassw: ex_bne.append((f.unit,b[1],f.demangled,n))

print("=== tight dispatch ladders (>=3 compares of one register, each with its own branch) ===")
print("  shape         source-has-switch   n")
for k,v in sorted(t.items(), key=lambda x:-x[1]):
    print("   %-13s %-19s %d" % (k[0],k[1],v))
tot_beq=t[("BEQ-LADDER",True)]+t[("BEQ-LADDER",False)]
tot_bne=t[("BNE-CHAIN",True)]+t[("BNE-CHAIN",False)]
if tot_beq: print("  BEQ-LADDER => switch precision: %.0f%%" % (100.0*t[("BEQ-LADDER",True)]/tot_beq))
if tot_bne: print("  BNE-CHAIN  => no-switch precision: %.0f%%" % (100.0*t[("BNE-CHAIN",False)]/tot_bne))
print()
print("=== BEQ-LADDER with NO switch in the source (%d) ===" % len(ex_beq))
for e in ex_beq[:20]: print("   %s:%d %s (%d compares)" % e)
print()
print("=== BNE-CHAIN with a switch in the source (%d) ===" % len(ex_bne))
for e in ex_bne[:20]: print("   %s:%d %s (%d compares)" % e)
