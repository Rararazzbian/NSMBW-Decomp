import json

with open('slices/d_basesNP.json') as f:
    slice_data = json.load(f)

meta_sections = slice_data['meta']['sections']
slices = slice_data['slices']

candidates = [
    {
        'name': 'd_a_wm_ghost.cpp',
        'class': 'daWmGhost_c',
        'sections': {
            '.text': (0x163620, 0x164230),
            '.ctors': (0x3E0, 0x3E4),
            '.rodata': (0x8880, 0x88B8),
            '.data': (0x44A9C, 0x44CB4),
            '.bss': (0xFDC0, 0xFDD0)
        }
    },
    {
        'name': 'd_a_wm_grid.cpp',
        'class': 'daWmGrid_c',
        'sections': {
            '.text': (0x164230, 0x164430),
            '.ctors': (0x3E4, 0x3E8),
            '.rodata': (0x88B8, 0x88D0),
            '.data': (0x44CB4, 0x44D54),
            '.bss': (0xFDD0, 0xFDE0)
        }
    },
    {
        'name': 'd_a_wm_kinoko_base.cpp',
        'class': 'daWmKinokoBase_c',
        'sections': {
            '.text': (0x16B2D0, 0x16BDA0),
            '.ctors': (0x3FC, 0x400),
            '.rodata': (0x8B70, 0x8BA8),
            '.data': (0x458E4, 0x45AB4),
            '.bss': (0xFE88, 0xFEA0)
        }
    },
    {
        'name': 'd_a_wm_kinoko_1up.cpp',
        'class': 'daWmKinoko1up_c',
        'sections': {
            '.text': (0x16B0F0, 0x16B2D0),
            '.ctors': (0x3F8, 0x3FC),
            '.rodata': (0x8B58, 0x8B70),
            '.data': (0x457EC, 0x458E4),
            '.bss': (0xFE78, 0xFE88)
        }
    },
    {
        'name': 'd_a_wm_boss_base.cpp',
        'class': 'daWmBossBase_c',
        'sections': {
            '.text': (0x189AC0, 0x18A260),
            '.ctors': (0x454, 0x458),
            '.rodata': (0x9590, 0x95D8),
            '.data': (0x485FC, 0x488C8),
            '.bss': (0x10B48, 0x10B60)
        }
    },
    {
        'name': 'd_a_wm_boss_larry.cpp',
        'class': 'daWmBossLarry_c',
        'sections': {
            '.text': (0x18B470, 0x18B690),
            '.ctors': (0x464, 0x468),
            '.rodata': (0x96B8, 0x96D0),
            '.data': (0x48CD8, 0x48E58),
            '.bss': (0x10BD0, 0x10BE0)
        }
    },
    {
        'name': 'd_a_wm_smallcloud.cpp',
        'class': 'daWmSmallCloud_c',
        'sections': {
            '.text': (0x1797E0, 0x179FF0),
            '.ctors': (0x430, 0x434),
            '.rodata': (0x8F58, 0x8FA0),
            '.data': (0x4728C, 0x47484),
            '.bss': (0x10130, 0x10140)
        }
    },
    {
        'name': 'd_a_wm_tower.cpp',
        'class': 'daWmTower_c',
        'sections': {
            '.text': (0x185710, 0x185B70),
            '.ctors': (0x44C, 0x450),
            '.rodata': (0x9488, 0x94A0),
            '.data': (0x480B4, 0x4818C),
            '.bss': (0x10A98, 0x10AA8)
        }
    }
]

def find_neighbors(sec_name, s_val, e_val):
    overlaps = []
    before = None
    after = None
    max_before_end = -1
    min_after_start = 0x7FFFFFFF

    for sl in slices:
        mr = sl.get('memoryRanges', {})
        if sec_name in mr:
            r = mr[sec_name]
            sl_s, sl_e = [int(x, 16) for x in r.split('-')]

            if max(s_val, sl_s) < min(e_val, sl_e):
                overlaps.append((sl.get('source', sl.get('sliceName')), sl_s, sl_e))

            if sl_e <= s_val:
                if sl_e > max_before_end:
                    max_before_end = sl_e
                    before = (sl.get('source', sl.get('sliceName')), sl_s, sl_e)

            if sl_s >= e_val:
                if sl_s < min_after_start:
                    min_after_start = sl_s
                    after = (sl.get('source', sl.get('sliceName')), sl_s, sl_e)

    return overlaps, before, after

print("=== OVERLAP AND ADJACENCY AUDIT FOR 8 d_basesNP CANDIDATES ===")
for cand in candidates:
    print(f"\nTU: {cand['name']} ({cand['class']})")
    for sec, (s_val, e_val) in cand['sections'].items():
        overlaps, before, after = find_neighbors(sec, s_val, e_val)
        print(f"  {sec}: 0x{s_val:x}-0x{e_val:x} (size 0x{e_val-s_val:x})")
        print(f"    Overlaps: {overlaps}")
        if before:
            gap_b = s_val - before[2]
            print(f"    Before: {before[0]} [0x{before[1]:x}-0x{before[2]:x}] (gap 0x{gap_b:x})")
        if after:
            gap_a = after[1] - e_val
            print(f"    After:  {after[0]} [0x{after[1]:x}-0x{after[2]:x}] (gap 0x{gap_a:x})")

