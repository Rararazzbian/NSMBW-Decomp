import os, sys
sys.path.insert(0, "wip/line_mng_shared")
import tally

d = tally.parse("wip/line_mng_merge/_tally/d.txt")
t = tally.parse(tally.TARGET)

pairs = [
    ("fn_800C15B0", "setArrElem_800C15B0"),  # won't be found - confirmed absent
    ("fn_800C1EE0", "fn_800C1EE0__FP10dLineMng_cffRC7mVec2_cRC7mVec2_cRC7mVec2_cRC7mVec2_c"),
    ("fn_800C3B20", "fn_800C3B20__FP10dLineMng_c"),
    ("fn_800C3B60", "fn_800C3B60__FP10dLineMng_c"),
]
for tgt_key, draft_key in pairs:
    tgt = t.get(tgt_key)
    draft = d.get(draft_key)
    if tgt is None:
        print(f"{tgt_key}: NOT IN TARGET?!"); continue
    if draft is None:
        print(f"{tgt_key}: draft key {draft_key!r} NOT FOUND -- genuinely absent (target {len(tgt)}w)")
        continue
    same_len = len(tgt) == len(draft)
    ok = tally.matched(draft, tgt)
    print(f"{tgt_key}: target {len(tgt)}w draft {len(draft)}w  len_ok={same_len}  matched(bytes/canon)={ok}")
    if not ok:
        # show first few differing lines (raw bytes)
        for i,((tb,tt),(db,dt)) in enumerate(zip(tgt, draft)):
            if tb != db:
                print(f"    first byte diff @ word {i}: target={tb!r} {tt!r}  draft={db!r} {dt!r}")
                break
