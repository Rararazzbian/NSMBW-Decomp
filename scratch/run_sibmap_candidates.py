import sys
import os
import re
import json
import subprocess
from pathlib import Path
from collections import defaultdict

sys.path.append('tools')
import sibmap

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
DIS = os.path.join(ROOT, 'tools', 'dis')

# Let's inspect the family warning in sibmap
corpus = sibmap.load_corpus()
print(f"Loaded corpus: {len(corpus)} functions")

# Let's select 10 strong candidate units in d_basesNP:
# 1. d_a_wm_ghost.cpp (g_profile_WM_GHOST, 0x163620-0x164230, 13 fns, adjacent to dokan_route)
# 2. d_a_wm_grid.cpp (g_profile_WM_GRID, 0x164230-0x164430, 10 fns, adjacent to wm_ghost)
# 3. d_a_wm_kinoko_base.cpp (g_profile_WM_KINOKO_BASE, 0x16B2D0-0x16BDA0, 17 fns, base for 1UP/Red/Star)
# 4. d_a_wm_kinoko_1up.cpp (g_profile_WM_KINOKO_1UP, 0x16B0F0-0x16B2D0, 9 fns, leaf derived from kinoko_base)
# 5. d_a_wm_boss_base.cpp (g_profile_WM_BOSS_BASE, 0x189AC0-0x18A260, 14 fns, base for all 7 koopaling icons)
# 6. d_a_wm_boss_larry.cpp (g_profile_WM_BOSS_LARRY, 0x18B470-0x18B690, 8 fns, leaf derived from boss_base)
# 7. d_a_wm_smallcloud.cpp (g_profile_WM_SMALLCLOUD, 0x1797E0-0x179FF0, 16 fns, twin of landed wm_cloud)
# 8. d_a_wm_tower.cpp (g_profile_WM_TOWER, 0x185710-0x185B70, 11 fns, leaf landmark)
# 9. d_a_wm_item.cpp (g_profile_WM_ITEM, 0x167120-0x167940, 12 fns)
# 10. d_a_wm_stop.cpp (g_profile_WM_STOP, 0x17AFF0-0x17B330, 10 fns)

candidates = [
    {
        'name': 'd_a_wm_ghost.cpp',
        'profile': 'g_profile_WM_GHOST',
        'class': 'daWmGhost_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x163620, 0x164230),
        'data': (0x44A9C, 0x44CB4),
        'rodata': (0x8880, 0x88B8),
        'ctors': (0x3E0, 0x3E4),
        'bss': (0xFDC0, 0xFDD0)
    },
    {
        'name': 'd_a_wm_grid.cpp',
        'profile': 'g_profile_WM_GRID',
        'class': 'daWmGrid_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x164230, 0x164430),
        'data': (0x44CB4, 0x44D54),
        'rodata': (0x88B8, 0x88D0),
        'ctors': (0x3E4, 0x3E8),
        'bss': (0xFDD0, 0xFDE0)
    },
    {
        'name': 'd_a_wm_kinoko_base.cpp',
        'profile': 'g_profile_WM_KINOKO_BASE',
        'class': 'daWmKinokoBase_c',
        'base_class': 'dWmObjActor_c',
        'is_base': True,
        'unblocks': 'daWmKinoko1up_c, daWmKinokoRed_c, daWmKinokoStar_c (3 derived TUs)',
        'text': (0x16B2D0, 0x16BDA0),
        'data': (0x458E4, 0x45AB4),
        'rodata': (0x8B70, 0x8BA8),
        'ctors': (0x3FC, 0x400),
        'bss': (0xFE88, 0xFEA0)
    },
    {
        'name': 'd_a_wm_kinoko_1up.cpp',
        'profile': 'g_profile_WM_KINOKO_1UP',
        'class': 'daWmKinoko1up_c',
        'base_class': 'daWmKinokoBase_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x16B0F0, 0x16B2D0),
        'data': (0x457EC, 0x458E4),
        'rodata': (0x8B58, 0x8B70),
        'ctors': (0x3F8, 0x3FC),
        'bss': (0xFE78, 0xFE88)
    },
    {
        'name': 'd_a_wm_boss_base.cpp',
        'profile': 'g_profile_WM_BOSS_BASE',
        'class': 'daWmBossBase_c',
        'base_class': 'dWmDemoActor_c',
        'is_base': True,
        'unblocks': 'daWmBossIggy_c, daWmBossLarry_c, daWmBossLemmy_c, daWmBossLudwig_c, daWmBossMorton_c, daWmBossRoy_c, daWmBossWendy_c (7 derived TUs)',
        'text': (0x189AC0, 0x18A260),
        'data': (0x485FC, 0x488C8),
        'rodata': (0x9590, 0x95D8),
        'ctors': (0x454, 0x458),
        'bss': (0x10B48, 0x10B60)
    },
    {
        'name': 'd_a_wm_boss_larry.cpp',
        'profile': 'g_profile_WM_BOSS_LARRY',
        'class': 'daWmBossLarry_c',
        'base_class': 'daWmBossBase_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x18B470, 0x18B690),
        'data': (0x48CD8, 0x48E58),
        'rodata': (0x96B8, 0x96D0),
        'ctors': (0x464, 0x468),
        'bss': (0x10BD0, 0x10BE0)
    },
    {
        'name': 'd_a_wm_smallcloud.cpp',
        'profile': 'g_profile_WM_SMALLCLOUD',
        'class': 'daWmSmallCloud_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x1797E0, 0x179FF0),
        'data': (0x4728C, 0x47484),
        'rodata': (0x8F58, 0x8FA0),
        'ctors': (0x430, 0x434),
        'bss': (0x10130, 0x10140)
    },
    {
        'name': 'd_a_wm_tower.cpp',
        'profile': 'g_profile_WM_TOWER',
        'class': 'daWmTower_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x185710, 0x185B70),
        'data': (0x480B4, 0x4818C),
        'rodata': (0x9488, 0x94A0),
        'ctors': (0x44C, 0x450),
        'bss': (0x10A98, 0x10AA8)
    },
    {
        'name': 'd_a_wm_item.cpp',
        'profile': 'g_profile_WM_ITEM',
        'class': 'daWmItem_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x167120, 0x167940),
        'data': (0x45030, 0x45270),
        'rodata': (0x89D0, 0x8A10),
        'ctors': (0x3EC, 0x3F0),
        'bss': (0xFE10, 0xFE28)
    },
    {
        'name': 'd_a_wm_stop.cpp',
        'profile': 'g_profile_WM_STOP',
        'class': 'daWmStop_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x17AFF0, 0x17B330),
        'data': (0x47530, 0x475EC),
        'rodata': (0x9040, 0x9068),
        'ctors': (0x434, 0x438),
        'bss': (0x10200, 0x10210)
    }
]

# Let's score each candidate against the corpus!
# We can disassemble the candidate ranges from original/d_basesNP.rel or check functions
print("\nScoring candidates against sibmap corpus...")
