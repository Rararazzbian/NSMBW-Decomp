import os
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import sibmap
from auto_decomp import harness

scratch_dir = os.path.join(ROOT, 'scratch', 'codex_round14')

# Target functions:
target_files = ['auto_00_001856E4_text.txt', 'auto_fn_2_185AC0_text.txt']
target_fns = []
for f in target_files:
    p = os.path.join(scratch_dir, f)
    fns = sibmap.parse(p)
    for fn in fns:
        if 0x185710 <= fn['addr'] < 0x185b70:
            target_fns.append(fn)

draft_fns = sibmap.parse(os.path.join(scratch_dir, 'd_a_wm_tower_compiled.txt'))

# Match functions by symbol / shape
print("=== Mapping Draft Functions to Target Functions in d_a_wm_tower.cpp ===")
matched_pairs = [
    (0x185710, "daWmTower_c_classInit__Fv"),
    (0x185740, "__ct__11daWmTower_cFv"),
    (0x1857a0, "__dt__11daWmTower_cFv"),
    (0x185840, "create__11daWmTower_cFv"),
    (0x1858a0, "execute__11daWmTower_cFv"),
    (0x185920, "draw__11daWmTower_cFv"),
    (0x185950, "doDelete__11daWmTower_cFv"),
    (0x185960, "createModel__11daWmTower_cFv"),
    (0x185a10, "calcModel__11daWmTower_cFv"),
    (0x185ac0, "__sinit_\\d_a_wm_tower_cpp"),
    (0x185b50, "__dt__Q26dWmLib19ForceInCourseList_tFv")
]

for addr, dname in matched_pairs:
    t = [f for f in target_fns if f['addr'] == addr]
    t = t[0] if t else None
    d = [f for f in draft_fns if f['name'] == dname or (dname.startswith('__sinit') and 'sinit' in f['name'])]
    d = d[0] if d else None
    
    if not t or not d:
        print(f"Missing: 0x{addr:x} ({dname}) - T:{bool(t)} D:{bool(d)}")
        continue
        
    t_canon = harness.canonicalise(t['texts'])
    d_canon = harness.canonicalise(d['texts'])
    is_match = (t_canon == d_canon)
    status = "MATCH" if is_match else f"DIFF ({len(t_canon)} vs {len(d_canon)})"
    print(f"0x{addr:06x} | Target: {len(t['words']):3d} insns | Draft: {len(d['words']):3d} insns | {status} | {dname}")
    if not is_match:
        for idx, (tl, dl) in enumerate(zip(t_canon, d_canon)):
            if tl != dl:
                print(f"    Line {idx:2d}: T: {tl} | D: {dl}")
