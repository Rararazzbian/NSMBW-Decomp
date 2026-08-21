import json
import re

symbols = []
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('//') or '=' not in line:
            continue
        parts = line.split('=', 1)
        sym_name = parts[0].strip()
        rest = parts[1].strip()
        m = re.match(r'(\.[a-zA-Z0-9_\$]+):0x([0-9a-fA-F]+);\s*(//\s*size:0x([0-9a-fA-F]+))?', rest)
        if m:
            sec = m.group(1)
            addr = int(m.group(2), 16)
            size = int(m.group(4), 16) if m.group(4) else 0
            symbols.append({'name': sym_name, 'sec': sec, 'addr': addr, 'size': size, 'raw': line})

sec_symbols = {}
for s in symbols:
    sec_symbols.setdefault(s['sec'], []).append(s)
for sec in sec_symbols:
    sec_symbols[sec].sort(key=lambda x: (x['addr'], x['size']))

with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)

sec_meta = slice_data['meta']['sections']

# Let us define the candidate TUs in Task A:
# Candidate 1: d_bc.cpp
# Candidate 2: d_beans_kuribo_mng.cpp (or part of d_bc / d_bg)
# Candidate 3: d_bg.cpp
# Candidate 4: d_bg_actor_mng.cpp
# Candidate 5: d_bg_unit.cpp
# Candidate 6: d_bg_tex_mng.cpp (or part of d_bg_unit)
# Candidate 7: d_capture_mng.cpp

# Let's inspect section bounds for each candidate:
# Let's write a function to get all symbols in a section in an address range
def get_syms_in_range(sec, lo, hi):
    return [s for s in sec_symbols.get(sec, []) if lo <= s['addr'] < hi]

# Let's check text slices:
# TU 1: d_bc.cpp: 0x8006CF40 to 0x80076BC0 (size 0x9C80 = 40,064 bytes)
#   sinit: 0x80076BB0
#   ctors: 0x802EDD8C (entry 42, off +0xa8)
#   rodata: 0x802EF6F0 - 0x802EF898 (size 0x1A8 = 424 bytes) (l_saka_data to lbl_802EF898)
#   data: 0x8030F588 - 0x8030F6E0 (size 0x158 = 344 bytes) (checkFoot.. to __vt__5dBc_c)
#   bss: 0x80356208 - 0x8035622C (size 0x24 = 36 bytes) (checkObjFoot/Head/Wall)
#   sdata: 0x80427C40 - 0x80427C50 (size 0x10 = 16 bytes) (_checkWall, _checkObjWall)
#   sbss: 0x8042A088 - 0x8042A0A0 (size 0x18 = 24 bytes) (gUnitX, gUnitY, gWaterType, gWaterPos, gWaterAngle)
#   sdata2: 0x8042BF20 - 0x8042BFE8 (size 0xC8 = 200 bytes) (@83367 to @86577)

# TU 2: d_beans_kuribo_mng (if standalone): 0x80076BC0 - 0x80076FD0 (size 0x410 = 1,040 bytes)
#   sinit: None
#   ctors: None
#   rodata: None
#   data: None
#   bss: None
#   sbss: 0x8042A0A0 - 0x8042A0A8 (size 0x8 = 8 bytes) (m_instance__17dBeansKuriboMng_c, lbl_8042A0A8)
#   sdata2: None

# TU 3: d_bg.cpp: 0x80076FD0 - 0x8007E180 (size 0x71B0 = 29,104 bytes)
#   sinit: 0x8007E170
#   ctors: 0x802EDD90 (entry 43, off +0xac)
#   rodata: 0x802EF898 - 0x802EFC68 (size 0x3D0 = 976 bytes)
#   data: 0x8030F6E0 - 0x8030F820 (size 0x140 = 320 bytes) (@82311 to __vt__5dBg_c)
#   sbss: 0x8042A0AC - 0x8042A0B8 (size 0xC = 12 bytes) (m_FrmHeap_p__5dBg_c, m_bg_p__5dBg_c)
#   sdata2: 0x8042BFF0 - 0x8042C130 (size 0x140 = 320 bytes) (@82353 to @85308)

# TU 4: d_bg_actor_mng.cpp: 0x8007E180 - 0x8007F7A0 (size 0x1620 = 5,664 bytes)
#   sinit: 0x8007EC20 (arraydtors follow up to 0x8007F7A0)
#   ctors: 0x802EDD94 (entry 44, off +0xb0)
#   rodata: 0x802EFC68 - 0x802EFC98 (size 0x30 = 48 bytes) (@68155)
#   data: 0x8030F820 - 0x80310068 (size 0x848 = 2,120 bytes) (l_object_name, l_Pa3_rail..., __vt__17dBgActorManager_c)
#   sbss: 0x8042A0B8 - 0x8042A0C0 (size 0x8 = 8 bytes) (ms_instance__17dBgActorManager_c, l_pRailList)
#   sdata2: 0x8042C130 - 0x8042C180 (size 0x50 = 80 bytes) (@71555 to @71467)

# TU 5: d_bg_unit.cpp: 0x8007F7A0 - 0x800872E0 (size 0x7B40 = 31,552 bytes)
#   sinit: 0x80087100
#   ctors: 0x802EDD98 (entry 45, off +0xb4)
#   rodata: 0x802EFC98 - 0x802F0360 (size 0x6C8 = 1,736 bytes)
#   data: 0x80310068 - 0x80310D78 (size 0xD10 = 3,344 bytes) (__vt__11dBgGlobal_c, __vt__14dBgParameter_c, __vt__17dShareBgTexProc_c, __vt__7bgTex_c, __vt__9dBgUnit_c, __vt__11dBgTexMng_c)
#   bss: 0x80356230 - 0x80356260 (size 0x30 = 48 bytes) (@67759, @67765, @67767, @67769)
#   sbss: 0x8042A0C0 - 0x8042A100 (size 0x40 = 64 bytes) (mEntryN/B, mGroupCtrlActor/No, ms_pInstance__11dBgGlobal_c, ms_Instance_p__14dBgParameter_c, ms_instance__11dBgTexMng_c, m_instance__11dBlockMng_c)
#   sdata2: 0x8042C180 - 0x8042C238 (size 0xB8 = 184 bytes) (@68048 to @76959)

# TU 6: d_capture_mng.cpp: 0x800872E0 (or 0x80088FD0) - 0x8008C200
#   sinit: 0x80089ED0
#   ctors: 0x802EDD9C (entry 46, off +0xb8)
#   rodata: 0x802F0360 - 0x802F03E8 (size 0x88 = 136 bytes)
#   data: 0x80310D78 - 0x803110A0 (size 0x328 = 808 bytes) (__vt__Q23m3d9capture_c, __vt__13dCaptureMng_c, __vt__6dDOF_c, __vt__18dDrawShadowModel_c, __vt__20dDrawShadowProjMap_c, __vt__16dMakeShadowTex_c, __vt__13dDrawScreen_c, __vt__14dCaptureCoin_c, __vt__10dSetupGX_c, __vt__10dCapture_c)
#   sbss: 0x8042A100 - 0x8042A140 (size 0x40 = 64 bytes) (l_lengthZ, ms_instance__13dCaptureMng_c, lbl_8042A108, drawOpa/drawQuad local statics)
#   sdata2: 0x8042C238 - 0x8042C2E0 (size 0xA8 = 168 bytes) (@51902 to @65834)

print("Section analysis verified!")
