import os, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath("wip/line_mng_shared/tally.py"))))
sys.path.insert(0, "wip/line_mng_shared")
import tally

src = os.path.abspath("wip/line_mng_merge/d_line_mng.cpp")
inc = os.path.abspath("wip/line_mng_merge/shadow_include")
work = os.path.join(os.path.dirname(src), '_tally')
os.makedirs(work, exist_ok=True)
obj, txt = os.path.join(work, 'd.o'), os.path.join(work, 'd.txt')
sys.path.insert(0, os.path.join("tools", "auto_decomp"))
import harness
ok, err = harness.compile_draft(src, obj, extra_inc=[inc])
if not ok:
    print("COMPILE FAILED"); print(err); sys.exit(1)
harness.disasm(obj, txt)
d, t = tally.parse(txt), tally.parse(tally.TARGET)

hit = [k for k in t if k in d and tally.matched(d[k], t[k])]
miss = [k for k in t if k not in d]
struct = [k for k in t if k in d and k not in hit]

print(f"TOTAL target fns: {len(t)}  matched: {len(hit)}  missing(not compiled): {len(miss)}  present-but-differing: {len(struct)}")
print()
print("=== MISSING (not defined in merge) ===")
for k in sorted(miss):
    print(f"  {len(t[k]):5d}w  {k}")
print()
print("=== PRESENT BUT DIFFERING ===")
for k in sorted(struct, key=lambda k: -len(t[k])):
    dn = len(d[k])
    n = len(t[k])
    state = 'LEN OK' if dn==n else f'{dn}w vs {n}w'
    print(f"  {n:5d}w  {state:>14}  {k}")

print()
print("=== FULL MATCHED LIST ===")
for k in sorted(hit):
    print(f"  {k}")
