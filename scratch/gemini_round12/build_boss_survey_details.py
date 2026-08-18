import os
import sys
import json

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from relfile import Rel

with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'boss_scored_tus.json')) as f:
    scored_tus = json.load(f)

with open(os.path.join(ROOT, 'original', 'd_en_bossNP.rel'), 'rb') as f:
    rel = Rel(4, file=f)

# Top 8 candidate indices (1-indexed):
# 1. TU 11 (g_profile_EN_BOSS_KOOPA_DEMO_CAGE) - 71.7% exact / 74.2% shape, 3368 B code (Zero-hazard starter)
# 2. TU 14 (g_profile_EN_BOSS_KOOPA_JR_A) - 46.0% exact / 51.7% shape, 6784 B code (Airship Jr starter, unblocks JR_B and JR_C)
# 3. TU 15 (g_profile_EN_BOSS_KOOPA_JR_B) - 48.9% exact / 55.8% shape, 8520 B code (Direct twin of JR_A)
# 4. TU 2 (g_profile_EN_BOSS_CASTLE_LARRY) - 44.1% exact / 56.2% shape, 7876 B code (Smallest Castle Koopaling)
# 5. TU 17 (g_profile_EN_BOSS_LARRY) - 45.7% exact / 60.7% shape, 11796 B code (Smallest World Boss Koopaling)
# 6. TU 1 (g_profile_EN_BOSS_CASTLE_IGGY) - 46.9% exact / 50.3% shape, 16328 B code (Base dEnTorideKokoopa_c, gates all 7 castle koopas)
# 7. TU 20 (g_profile_EN_BOSS_MORTON) - 44.3% exact / 53.7% shape, 14668 B code (Ground-pound physics boss)
# 8. TU 23 (No Profile, Warning Layout / LytBase Manager) - 40.1% exact / 45.7% shape, 6208 B code (Small helper/layout TU)

# Let's verify exact section bounds for all 26 TUs from rel relocations and data layout
# In d_en_bossNP.rel:
# ctors is 0x0 to 0x68 (26 slots: 4 bytes each)
# Let's print full details for the top candidates

top_candidates = [11, 14, 15, 2, 17, 1, 20, 23]

print("=== Top 8 Candidate Boss TUs Detailed Analysis ===")
for rank, tu_idx in enumerate(top_candidates, 1):
    tu = [t for t in scored_tus if t['index'] == tu_idx][0]
    p_name = tu['profiles'][0][1] if tu['profiles'] else "d_boss_warning.cpp (LytBase_c / Warning Manager)"
    print(f"\nRank {rank}: TU {tu['index']} — {p_name}")
    print(f"  .text: 0x{tu['text_start']:05x}-0x{tu['text_end']:05x} (Span: {tu['span_bytes']} B, Code: {tu['code_bytes']} B, {tu['fn_count']} functions)")
    print(f"  .ctors: 0x{(tu_idx-1)*4:02x}-0x{tu_idx*4:02x} (Span: 4 B)")
    print(f"  Sibling score: {tu['exact_sibling_score']:.2f}% exact / {tu['shape_sibling_score']:.2f}% shape")
    print(f"  Symbol coverage: {tu['named_fn_count']} named, {tu['anon_fn_count']} anonymous ({tu['anon_fn_count']/tu['fn_count']*100:.1f}% anonymous)")
