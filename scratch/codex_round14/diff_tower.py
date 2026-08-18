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

print(f"Loaded {len(target_fns)} target functions in d_a_wm_tower.cpp")

# Draft functions:
draft_fns = sibmap.parse(os.path.join(scratch_dir, 'd_a_wm_tower_compiled.txt'))
print(f"Loaded {len(draft_fns)} draft functions in d_a_wm_tower.cpp")

print("\n=== Per-Function Comparison for d_a_wm_tower.cpp ===")
for i, t in enumerate(target_fns):
    d = draft_fns[i] if i < len(draft_fns) else None
    t_insns = len(t['words'])
    d_insns = len(d['words']) if d else 0
    t_name = t['name']
    d_name = d['name'] if d else "None"
    
    # Check if instructions match
    is_match = False
    diff_text = ""
    if d:
        t_canon = harness.canonicalise(t['texts'])
        d_canon = harness.canonicalise(d['texts'])
        if t_canon == d_canon:
            is_match = True
        else:
            diff_text = f"Diff in {len(t_canon)} vs {len(d_canon)} lines"
            
    status = "MATCH" if is_match else f"DIFF ({t_insns} target vs {d_insns} draft)"
    print(f"[{i+1:2d}] 0x{t['addr']:06x} | Target: {t_insns:3d} insns | Draft: {d_insns:3d} insns | {status} | {t_name} <-> {d_name}")
    if not is_match and d:
        for line_idx, (tl, dl) in enumerate(zip(t_canon, d_canon)):
            if tl != dl:
                print(f"      Line {line_idx}: Target: {tl} | Draft: {dl}")
