import os
import sys
import json
import re

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

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

    # Top 8 candidate indices (1-indexed):
    # 1. TU 11: g_profile_EN_BOSS_KOOPA_DEMO_CAGE
    # 2. TU 14: g_profile_EN_BOSS_KOOPA_JR_A
    # 3. TU 15: g_profile_EN_BOSS_KOOPA_JR_B
    # 4. TU 2:  g_profile_EN_BOSS_CASTLE_LARRY
    # 5. TU 17: g_profile_EN_BOSS_LARRY
    # 6. TU 1:  g_profile_EN_BOSS_CASTLE_IGGY
    # 7. TU 20: g_profile_EN_BOSS_MORTON
    # 8. TU 23: d_boss_warning.cpp

    # Let's inspect data boundaries for all 8 candidates
    # Data profile positions:
    # TU 1:  0x0 - 0xeb0
    # TU 2:  0xeb0 - 0x1958
    # TU 3:  0x1958 - 0x24a0
    # TU 4:  0x24a0 - 0x3220
    # TU 5:  0x3220 - 0x3eb8
    # TU 6:  0x3eb8 - 0x4e90
    # TU 7:  0x4e90 - 0x5a40
    # TU 8:  0x5a40 - 0x6ae8
    # TU 9:  0x6ae8 - 0x7d60
    # TU 10: 0x7d60 - 0x9500
    # TU 11: 0x9500 - 0x9988
    # TU 12: 0x9988 - 0xa058
    # TU 13: 0xa058 - 0xa680
    # TU 14: 0xa680 - 0xad90
    # TU 15: 0xad90 - 0xb5d0
    # TU 16: 0xb5d0 - 0xbcd8
    # TU 17: 0xbcd8 - 0xcb98
    # TU 18: 0xcb98 - 0xde30
    # TU 19: 0xde30 - 0xf168
    # TU 20: 0xf168 - 0x10180
    # TU 21: 0x10180 - 0x11378
    # TU 22: 0x11378 - 0x1252c
    # TU 23: 0x1252c - 0x12dc0
    # TU 24: 0x12dc0 - 0x13108
    # TU 25: 0x13108 - 0x13470
    # TU 26: 0x13470 - 0x13520

    # Rodata boundaries:
    # Let's inspect rodata symbols
    rodata_syms = [s for s in boss_syms if s['sec'] == '.rodata']
    rodata_syms.sort(key=lambda s: s['addr'])
    
    # Bss boundaries:
    bss_syms = [s for s in boss_syms if s['sec'] == '.bss']
    bss_syms.sort(key=lambda s: s['addr'])

    print("=== Section Bounds for Top 8 Candidate Boss Units ===")
    
    candidate_indices = [11, 14, 15, 2, 17, 1, 20, 23]
    
    for rank, tu_idx in enumerate(candidate_indices, 1):
        tu = [t for t in scored_tus if t['index'] == tu_idx][0]
        p_name = tu['profiles'][0][1] if tu['profiles'] else "d_boss_warning.cpp"
        print(f"\n--- Candidate {rank}: TU {tu_idx} ({p_name}) ---")
        print(f"  .text: 0x{tu['text_start']:x}-0x{tu['text_end']:x}")
        print(f"  .ctors: 0x{(tu_idx-1)*4:x}-0x{tu_idx*4:x}")
        print(f"  Code bytes: {tu['code_bytes']}, Span bytes: {tu['span_bytes']}, Functions: {tu['fn_count']}")
        print(f"  Sibling similarity: {tu['exact_sibling_score']:.2f}% exact / {tu['shape_sibling_score']:.2f}% shape")
        print(f"  Symbol coverage: {tu['named_fn_count']}/{tu['fn_count']} named ({tu['anon_fn_count']} anonymous)")

if __name__ == '__main__':
    main()
