import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sibmap_json_path = ROOT / 'scratch' / 'gemini_round9' / 'sibmap.json'

data = json.loads(sibmap_json_path.read_text())

total_bytes = 0
exact_matched_bytes = 0
shape_matched_bytes = 0

print(f"Total target functions analyzed in sibmap: {len(data)}")

function_stats = []

for fn in data:
    name = fn['name']
    insns = fn['insns']
    fn_bytes = fn['bytes']
    total_bytes += fn_bytes
    
    top_list = fn.get('top', [])
    top = top_list[0] if top_list else None
    
    top_exact = top['exact_sim'] if top else 0.0
    top_shape = top['shape_sim'] if top else 0.0
    top_tu = top['tu'] if top else 'NONE'
    top_fn = top['name'] if top else 'NONE'
    
    # Check family matches specifically
    fam_list = fn.get('famhits', [])
    top_fam = fam_list[0] if fam_list else None
    fam_exact = top_fam['exact_sim'] if top_fam else 0.0
    fam_shape = top_fam['shape_sim'] if top_fam else 0.0
    fam_tu = top_fam['tu'] if top_fam else 'NONE'
    fam_fn = top_fam['name'] if top_fam else 'NONE'
    
    exact_matched_bytes += fn_bytes * top_exact
    shape_matched_bytes += fn_bytes * top_shape
    
    function_stats.append({
        'name': name,
        'bytes': fn_bytes,
        'top_exact': top_exact,
        'top_shape': top_shape,
        'top_tu': top_tu,
        'top_fn': top_fn,
        'fam_exact': fam_exact,
        'fam_shape': fam_shape,
        'fam_tu': fam_tu,
        'fam_fn': fam_fn
    })

function_stats.sort(key=lambda x: x['top_exact'], reverse=True)

print(f"\nOverall Precedent Rate across all {total_bytes} bytes:")
print(f"  Global Exact Byte-Correspondence: {exact_matched_bytes / total_bytes * 100:.2f}% ({exact_matched_bytes:.1f} / {total_bytes} B)")
print(f"  Global Shape Byte-Correspondence: {shape_matched_bytes / total_bytes * 100:.2f}% ({shape_matched_bytes:.1f} / {total_bytes} B)")

print("\n--- DETAILED FUNCTION SIBLING MAPPING ---")
for f in function_stats:
    print(f"{f['name']:45s} ({f['bytes']:4d}B):")
    print(f"    Global: exact={f['top_exact']*100:5.1f}%, shape={f['top_shape']*100:5.1f}% | {f['top_tu']}::{f['top_fn']}")
    if f['fam_tu'] != 'NONE':
        print(f"    Family: exact={f['fam_exact']*100:5.1f}%, shape={f['fam_shape']*100:5.1f}% | {f['fam_tu']}::{f['fam_fn']}")
