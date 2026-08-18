import os
import sys
import json
import re
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from relfile import Rel

def load_symbols(path):
    syms = []
    sym_re = re.compile(r'^(\S+)\s*=\s*(\.\w+):0x([0-9a-fA-F]+);\s*(?://\s*(.*))?$')
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('//'):
                continue
            m = sym_re.match(line)
            if m:
                name = m.group(1)
                sec = m.group(2)
                addr = int(m.group(3), 16)
                meta = m.group(4) or ''
                size = 0
                sz_m = re.search(r'size:0x([0-9a-fA-F]+)', meta)
                if sz_m:
                    size = int(sz_m.group(1), 16)
                syms.append({'name': name, 'sec': sec, 'addr': addr, 'size': size})
    return syms

def main():
    boss_syms = load_symbols(os.path.join(ROOT, 'bin', 'dtk', 'd_en_bossNP_symbols.txt'))
    with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'boss_scored_tus.json')) as f:
        scored_tus = json.load(f)

    with open(os.path.join(ROOT, 'original', 'd_en_bossNP.rel'), 'rb') as f:
        rel = Rel(4, file=f)

    # All 26 TUs text split starts:
    split_starts = [
        0x110, 0x45dc, 0x666c, 0x9bc4, 0xde30, 0x11738, 0x156d0, 0x1807c,
        0x1c12c, 0x2102c, 0x29118, 0x29efc, 0x2beb4, 0x2dff8, 0x2fbf4,
        0x31e94, 0x34800, 0x377e8, 0x3d038, 0x42798, 0x4635c, 0x4a35c,
        0x4ee48, 0x50744, 0x51bc4, 0x55c94, 0x5702c
    ]

    # Data boundaries:
    # 26 TUs data ranges
    data_starts = [
        0x00000, 0x00eb0, 0x01958, 0x024a0, 0x03220, 0x03eb8, 0x04e90, 0x05a40,
        0x06ae8, 0x07d60, 0x09500, 0x09988, 0x0a058, 0x0a680, 0x0ad90, 0x0b5d0,
        0x0bcd8, 0x0cb98, 0x0de30, 0x0f168, 0x10180, 0x11378, 0x1252c, 0x12dc0,
        0x13108, 0x13470, 0x13520
    ]

    # Rodata boundaries:
    # Let's inspect rodata symbols and relocations
    # Rodata starts for 26 TUs:
    rodata_starts = [
        0x0000, 0x0140, 0x01f0, 0x02c8, 0x03e8, 0x05d8, 0x06a0, 0x0750,
        0x0a08, 0x0b50, 0x0cb0, 0x0cd8, 0x0d28, 0x0d60, 0x0e70, 0x0f88,
        0x10f0, 0x1328, 0x15b0, 0x1818, 0x1ad0, 0x1d18, 0x1f70, 0x2100,
        0x2120, 0x2168, 0x21f0
    ]

    # Bss boundaries:
    bss_starts = [
        0x0008, 0x0488, 0x0690, 0x0918, 0x0ca0, 0x1060, 0x1568, 0x1770,
        0x1bf0, 0x2280, 0x26c8, 0x2714, 0x2754, 0x2a28, 0x2b60, 0x2d60,
        0x2ea0, 0x30e8, 0x3670, 0x3bf8, 0x3f78, 0x43c0, 0x4808, 0x4818,
        0x4838, 0x4848, 0x487c
    ]

    print("=== Complete Section Bounds for Top 8 Boss Candidates ===")
    
    # Ranked order:
    # 1. TU 11: g_profile_EN_BOSS_KOOPA_DEMO_CAGE (daEnBossKoopaDemoCage_c)
    # 2. TU 14: g_profile_EN_BOSS_KOOPA_JR_A (daEnBossKoopaJrA_c / World 4 Airship Jr)
    # 3. TU 15: g_profile_EN_BOSS_KOOPA_JR_B (daEnBossKoopaJrB_c / World 6 Airship Jr)
    # 4. TU 2:  g_profile_EN_BOSS_CASTLE_LARRY (daEnBossCastleLarry_c / Tower Larry)
    # 5. TU 17: g_profile_EN_BOSS_LARRY (daEnBossLarry_c / Castle Larry)
    # 6. TU 1:  g_profile_EN_BOSS_CASTLE_IGGY (daEnBossCastleIggy_c / dEnTorideKokoopa_c base)
    # 7. TU 20: g_profile_EN_BOSS_MORTON (daEnBossMorton_c / Castle Morton)
    # 8. TU 23: d_boss_warning.cpp (LytBase_c / Warning layout manager)

    candidate_order = [
        (11, "d_a_en_boss_koopa_demo_cage.cpp", "g_profile_EN_BOSS_KOOPA_DEMO_CAGE", "daEnBossKoopaDemoCage_c", "Leaf (Demo Actor)", "Zero-risk starter; unlocks Koopa demo cluster"),
        (14, "d_a_en_boss_koopa_jr_a.cpp", "g_profile_EN_BOSS_KOOPA_JR_A", "daEnBossKoopaJrA_c", "Base / Lead Leaf", "Unblocks Koopa Jr. Airship series (Jr B and Jr C)"),
        (15, "d_a_en_boss_koopa_jr_b.cpp", "g_profile_EN_BOSS_KOOPA_JR_B", "daEnBossKoopaJrB_c", "Leaf", "Direct sibling twin of Koopa Jr. A (electric shock arena)"),
        (2,  "d_a_en_boss_castle_larry.cpp", "g_profile_EN_BOSS_CASTLE_LARRY", "daEnBossCastleLarry_c", "Leaf", "Smallest Castle Koopaling (7.8 KB code; high sibling match)"),
        (17, "d_a_en_boss_larry.cpp", "g_profile_EN_BOSS_LARRY", "daEnBossLarry_c", "Leaf", "Smallest World Boss Koopaling (11.8 KB code; 60.7% shape match)"),
        (1,  "d_a_en_boss_castle_iggy.cpp", "g_profile_EN_BOSS_CASTLE_IGGY", "daEnBossCastleIggy_c", "Base (dEnTorideKokoopa_c)", "High-leverage base; gates all 7 Castle Koopalings"),
        (20, "d_a_en_boss_morton.cpp", "g_profile_EN_BOSS_MORTON", "daEnBossMorton_c", "Leaf", "Pillar ground-pound physics boss (14.7 KB code)"),
        (23, "d_boss_warning.cpp", "No Profile (Layout Actor)", "dWarningManager_c / LytBase_c", "Helper / Layout", "Boss battle warning sirens and layout HUD animator")
    ]

    for rank, (tu_idx, src_name, p_name, cls_name, role_type, gating) in enumerate(candidate_order, 1):
        tu = [t for t in scored_tus if t['index'] == tu_idx][0]
        i = tu_idx - 1
        t_r = f"0x{split_starts[i]:x}-0x{split_starts[i+1]:x}"
        c_r = f"0x{i*4:x}-0x{(i+1)*4:x}"
        r_r = f"0x{rodata_starts[i]:x}-0x{rodata_starts[i+1]:x}"
        d_r = f"0x{data_starts[i]:x}-0x{data_starts[i+1]:x}"
        b_r = f"0x{bss_starts[i]:x}-0x{bss_starts[i+1]:x}"
        
        print(f"\n### Rank {rank}: `{src_name}` (TU {tu_idx})")
        print(f"- Profile & Class: `{p_name}` / `{cls_name}` ({role_type})")
        print(f"- Gating Impact: {gating}")
        print(f"- Section Bounds:")
        print(f"  * .text: `{t_r}` (Span: {tu['span_bytes']} B, Code: {tu['code_bytes']} B, {tu['fn_count']} functions)")
        print(f"  * .ctors: `{c_r}` (Span: 4 B)")
        print(f"  * .rodata: `{r_r}` (Span: {rodata_starts[i+1]-rodata_starts[i]} B)")
        print(f"  * .data: `{d_r}` (Span: {data_starts[i+1]-data_starts[i]} B)")
        print(f"  * .bss: `{b_r}` (Span: {bss_starts[i+1]-bss_starts[i]} B)")
        print(f"- Sibling Score: **{tu['exact_sibling_score']:.2f}% exact / {tu['shape_sibling_score']:.2f}% shape**")
        print(f"- Symbol Coverage: {tu['named_fn_count']}/{tu['fn_count']} named ({tu['anon_fn_count']} anonymous, {tu['anon_fn_count']/tu['fn_count']*100:.1f}% anonymous)")

if __name__ == '__main__':
    main()
