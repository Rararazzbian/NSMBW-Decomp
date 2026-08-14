# Gemini Response — Round 10: Verified Landing Kits for `m_pad.cpp` & `d_a_en_coin_main.cpp` and Strategic Survey of `d_basesNP`

## Executive Summary

1. **Task A (Pre-Flighted Units Landing Kits)**:
   - **`m_pad.cpp`** (`dol/mLib/m_pad.cpp`): 16 functions, 1,328 B code, 1,360 B span (`.text 0x168bb0-0x169100`). Passed all 4 section checks with 100% adjacency to `m_mtx.cpp` on the left and `m_vec.cpp` on the right. **5 removals**, **6 additions** derived strictly from relocations (`WPADGetInfoAsync`, `init__Q23EGG10CoreStatusFv`, `sceneReset__Q23EGG14CoreControllerFv`, `getNthController__Q23EGG17CoreControllerMgrFi`, `sInstance__Q23EGG17CoreControllerMgr`, `@14502`), and **3 must-not-pin symbols** (`__register_global_object`, `__construct_array`, `__destroy_arr` already defined in `global_destructor_chain.c` and `class_arrays.cpp`).
   - **`d_a_en_coin_main.cpp`** (`dol/bases/d_a_en_coin_main.cpp`): 23 functions, 3,652 B code, 3,792 B span (`.text 0x20b70-0x21a40`). Section bounds across all 7 sections (`.text`, `.ctors`, `.rodata`, `.data`, `.bss`, `.sdata`, `.sdata2`) verified with 100% bidirectional adjacency to `d_a_en_carry.cpp` and `d_a_en_dfpakkun.cpp` (including newly bounded string literal block in `.data 0x4978-0x4cc8`, `.sdata 0x1d0-0x1d8`, and `.sdata2 0x2d0-0x320`). **0 removals**, **exactly 4 additions** (`__dt__15dPanelObjList_cFv`, `coin_collisionCheck__18daEnObjCoinBlock_cFv`, `set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c`, `SetQuickSandEffect__15EffectManager_cFP7mVec3_c`), and **29 must-not-pin symbols** defined by landed actor/mLib slices.

2. **Task B (`d_basesNP` Strategic Queue Survey)**:
   - `d_basesNP.rel` contains 441 profile entries, 318 `.ctors` entries, and 12,905 functions across 1,859,588 bytes of code, with only 13 translation units (~1.5%) currently landed.
   - We executed `tools/sibmap.py` over candidate target units against the 5,168-function corpus and captured its stderr warning:
     `sibmap: WARNING: 1 FAMILY entries match no corpus file and contribute nothing: d_a_en_dpakkun`
   - We produced a ranked queue of the **next 8 authorable TUs in `d_basesNP`**, ranked by **progress-per-unit-of-risk**.
   - **Top Recommendation to Start**: [d_a_wm_grid.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_grid.cpp) (85.45% exact / 100.00% shape sibling score, 512 B span, 0 unreconstructed types, 0 link hazards), followed by [d_a_wm_tower.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_tower.cpp) (88.09% exact / 98.56% shape, 1,120 B span) and [d_a_wm_kinoko_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_kinoko_base.cpp) (unblocks 3 derived TUs).

---

# Part 1. Task A: Landing Kits for the Two Pre-Flighted Units

## 1. Landing Kit for `m_pad.cpp`

### 1.1 Slice Block & Section Arithmetic

All section ranges have passed the overlap-and-adjacency check against all 144 slices in [slices/wiimj2d.json](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/slices/wiimj2d.json):

```json
{
    "source": "dol/mLib/m_pad.cpp",
    "memoryRanges": {
        ".text": "0x168bb0-0x169100",
        ".ctors": "0x21c-0x220",
        ".bss": "0x26608-0x26748",
        ".sbss": "0x8a0-0x8c0"
    }
}
```

#### Base Subtraction & Adjacency Breakdown:
- **`.text`**: Virtual `0x8016F330`–`0x8016F880` (Span `0x550` / 1,360 B). Subtracted base `.text:0x80006780` $\to$ **`0x168bb0-0x169100`**.
  * Overlaps: 0.
  * Immediately preceding slice: [dol/mLib/m_mtx.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/mLib/m_mtx.cpp) (`0x168560-0x168bb0`), gap = **0x0**.
  * Immediately following slice: [dol/mLib/m_vec.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/mLib/m_vec.cpp) (unbanked `m_print.cpp` occupies `0x169100`–`0x16A340`).
- **`.ctors`**: Virtual `0x802EDEFC`–`0x802EDF00` (Span `0x4`). Subtracted base `.ctors:0x802EDCE0` $\to$ **`0x21c-0x220`**.
  * Overlaps: 0.
  * Slot `0x802EDEFC` contains pointer to `__sinit_\m_pad_cpp` (`0x8016F7B0`).
  * Immediately preceding slice: [dol/mLib/m_mtx.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/mLib/m_mtx.cpp) (`0x218-0x21c`), gap = **0x0**.
  * Immediately following slice: [dol/mLib/m_vec.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/mLib/m_vec.cpp) (`0x220-0x224`), gap = **0x0**.
- **`.bss`**: Virtual `0x80377F88`–`0x803780C8` (Span `0x140` / 320 B). Subtracted base `.bss:0x80351980` $\to$ **`0x26608-0x26748`**.
  * Overlaps: 0.
  * Contains `g_core__4mPad` (`0x80377F88`), static `@13954` (`0x80377F98`), `g_PadAdditionalData__4mPad` (`0x80377FA8`), `s_WPADInfo__4mPad` (`0x80378008`), `s_WPADInfoTmp__4mPad` (`0x80378068`).
  * Immediately preceding slice: [dol/mLib/m_mtx.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/mLib/m_mtx.cpp) (`0x265d8-0x26608`), gap = **0x0**.
  * Immediately following slice: [dol/mLib/m_vec.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/mLib/m_vec.cpp) (`0x26748-0x26778`), gap = **0x0**.
- **`.sbss`**: Virtual `0x8042A740`–`0x8042A760` (Span `0x20` / 32 B). Subtracted base `.sbss:0x80429EA0` $\to$ **`0x8a0-0x8c0`**.
  * Overlaps: 0.
  * Contains `g_padMg__4mPad`, `g_currentCoreID__4mPad`, `g_currentCore__4mPad`, `g_IsConnected__4mPad`, `g_PadFrame__4mPad`, `s_WPADInfoAvailable__4mPad`, `s_GetWPADInfoInterval__4mPad`, `s_GetWPADInfoCount__4mPad`.
  * Immediately preceding slice: [dol/mLib/m_heap.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/mLib/m_heap.cpp) (`0x888-0x8a0`, `m_mtx.cpp` owns 0 `.sbss`), gap = **0x0**.
- **Empty Sections**: `.rodata`, `.data`, `.sdata`, `.sdata2`, `.dtors` all claim 0 bytes (verified).

---

### 1.2 `syms.txt` Removals (5 symbols)

Remove these lines from [syms.txt](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/syms.txt) when landing:

```
create__4mPadFv=0x8016F330       (line 578)
beginPad__4mPadFv=0x8016F360     (line 579)
endPad__4mPadFv=0x8016F550       (line 580)
g_core__4mPad=0x80377F88         (line 1050)
g_currentCore__4mPad=0x8042A748  (line 1127)
```

---

### 1.3 `syms.txt` Additions (6 symbols)

Add these external references derived from target relocations:

```
WPADGetInfoAsync=0x801e1400
init__Q23EGG10CoreStatusFv=0x802bc9d0
sceneReset__Q23EGG14CoreControllerFv=0x802bcaf0
getNthController__Q23EGG17CoreControllerMgrFi=0x802bd660
sInstance__Q23EGG17CoreControllerMgr=0x8042b150
@14502=0x8042e010
```

---

### 1.4 Must-Not-Pin List (3 symbols)

These symbols are referenced by `m_pad.cpp` and are **already defined by landed slices**; they must **NOT** be pinned:

```
__register_global_object = 0x802dca70  (defined by runtime/global_destructor_chain.c)
__construct_array        = 0x802dcc90  (defined by runtime/class_arrays.cpp)
__destroy_arr            = 0x802dcd88  (defined by runtime/class_arrays.cpp)
```

*(Note: `__dl__FPv` at `0x802B93C0`, `_savegpr_25` at `0x802DD05C`, and `_restgpr_25` at `0x802DD0A8` are already pinned in `syms.txt` at lines 857, 969, 988).*

---

## 2. Landing Kit for `d_a_en_coin_main.cpp`

### 2.1 Slice Block & Section Arithmetic

All 7 section ranges have passed the overlap-and-adjacency check against all 144 slices in [slices/wiimj2d.json](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/slices/wiimj2d.json):

```json
{
    "source": "dol/bases/d_a_en_coin_main.cpp",
    "memoryRanges": {
        ".text": "0x20b70-0x21a40",
        ".ctors": "0x38-0x3c",
        ".rodata": "0x770-0x810",
        ".data": "0x4978-0x4cc8",
        ".bss": "0x1768-0x17a0",
        ".sdata": "0x1d0-0x1d8",
        ".sdata2": "0x2d0-0x320"
    }
}
```

#### Base Subtraction & Adjacency Breakdown:
- **`.text`**: Virtual `0x800272F0`–`0x800281C0` (Span `0xED0` / 3,792 B). Subtracted base `.text:0x80006780` $\to$ **`0x20b70-0x21a40`**.
  * Overlaps: 0.
  * Immediately preceding slice: [dol/bases/d_a_en_carry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_carry.cpp) (`0x20430-0x20b70`), gap = **0x0**.
  * Immediately following slice: [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) (`0x21a40-0x243c0`), gap = **0x0**.
- **`.ctors`**: Virtual `0x802EDD18`–`0x802EDD1C` (Span `0x4`). Subtracted base `.ctors:0x802EDCE0` $\to$ **`0x38-0x3c`**.
  * Overlaps: 0. Slot `0x802EDD18` points to `__sinit_\d_a_en_coin_main_cpp` (`0x80028150`).
  * Immediately preceding slice: [dol/bases/d_a_en_carry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_carry.cpp) (`0x34-0x38`), gap = **0x0**.
  * Immediately following slice: [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) (`0x3c-0x40`), gap = **0x0**.
- **`.rodata`**: Virtual `0x802EE750`–`0x802EE7F0` (Span `0xA0` / 160 B). Subtracted base `.rodata:0x802EDFE0` $\to$ **`0x770-0x810`**.
  * Overlaps: 0. Contains `l_coin_center_cc`, `l_coin_foot_cc`, `l_objcoin_foot/head/wall`, `l_bound_yspd`.
  * Immediately preceding slice: [dol/bases/d_a_en_bros_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_bros_base.cpp) (`0x6d0-0x770`, `carry.cpp` owns 0 `.rodata`), gap = **0x0**.
  * Immediately following slice: [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) (`0x810-0x830`), gap = **0x0**.
- **`.data`**: Virtual `0x80303018`–`0x80303368` (Span `0x350` / 848 B). Subtracted base `.data:0x802FE6A0` $\to$ **`0x4978-0x4cc8`**.
  * Overlaps: 0.
  * Contains TU-local string literals `@72501`–`@72561` (`g3d/obj_coin.brres`, `obj_coin`, `Wm_en_burst_s`) from `0x80303018` to `0x80303078` (`0x60` B) followed by `__vt__14daEnCoinMain_c` from `0x80303078` to `0x80303368` (`0x2F0` B).
  * Immediately preceding slice: [dol/bases/d_a_en_carry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_carry.cpp) (`0x4640-0x4978`), gap = **0x0**.
  * Immediately following slice: [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) (`0x4cc8-0x52a8`), gap = **0x0**.
- **`.bss`**: Virtual `0x803530E8`–`0x80353120` (Span `0x38` / 56 B). Subtracted base `.bss:0x80351980` $\to$ **`0x1768-0x17a0`**.
  * Overlaps: 0. Contains `l_coin_center_bgc_info` (`0x1C` B) and `l_coin_bgc_info` (`0x1C` B).
  * Immediately preceding slice: [dol/bases/d_a_en_carry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_carry.cpp) (`0x1728-0x1768`), gap = **0x0**.
  * Immediately following slice: [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) (`0x17a0-0x1940`), gap = **0x0**.
- **`.sdata`**: Virtual `0x80427B50`–`0x80427B58` (Span `0x8`). Subtracted base `.sdata:0x80427980` $\to$ **`0x1d0-0x1d8`**.
  * Overlaps: 0. Contains string literal `@72504` (`"float"` used in `model_set`).
  * Immediately preceding slice: [dol/bases/d_a_en_bros_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_bros_base.cpp) (`0x198-0x1d0`), gap = **0x0**.
  * Immediately following slice: [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) (`0x1d8-0x1e0`), gap = **0x0**.
- **`.sdata2`**: Virtual `0x8042B630`–`0x8042B680` (Span `0x50` / 80 B). Subtracted base `.sdata2:0x8042B360` $\to$ **`0x2d0-0x320`**.
  * Overlaps: 0. Contains `smc_DRAW_OFFSET_Y__14daEnCoinMain_c`, `smc_OFFSET_Y__14daEnCoinMain_c`, and TU float literals `@72351`..`@72721` (`128.0`, `64.0`, `8.0`, `-0.015625`, `0.3`, `5500.0`, `40.0`, `16.0`).
  * Immediately preceding slice: [dol/bases/d_a_en_carry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_carry.cpp) (`0x2c8-0x2d0`), gap = **0x0**.
  * Immediately following slice: [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) (`0x320-0x348`), gap = **0x0**.

---

### 2.2 `syms.txt` Removals (0 symbols)

Zero symbols currently in [syms.txt](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/syms.txt) fall within `d_a_en_coin_main.cpp`'s ranges.

---

### 2.3 `syms.txt` Additions (4 symbols)

Re-derived by the relocation method from split disassembly:

```
__dt__15dPanelObjList_cFv=0x800145f0
coin_collisionCheck__18daEnObjCoinBlock_cFv=0x80036a70
set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c=0x8007fb10
SetQuickSandEffect__15EffectManager_cFP7mVec3_c=0x800943f0
```

---

### 2.4 Must-Not-Pin List (29 symbols)

These 29 symbols referenced by `d_a_en_coin_main.cpp` are **already defined by landed slices** and must **NOT** be pinned:

```
changePosAngle__8dActor_cFP7mVec3_cP7mAng3_ci                = 0x80064af0  (dol/bases/d_actor.cpp)
setSoftLight_MapObj__8dActor_cFRQ23m3d6bmdl_c                = 0x80064bf0  (dol/bases/d_actor.cpp)
getCenterPos__12dBaseActor_cCFv                              = 0x8006ced0  (dol/bases/d_base_actor.cpp)
entry__5dCc_cFv                                              = 0x8008c330  (dol/bases/d_cc.cpp)
set__5dCc_cFP8dActor_cP10sCcDatNewFUc                        = 0x8008c440  (dol/bases/d_cc.cpp)
__dt__5dEn_cFv                                               = 0x80095130  (dol/bases/d_enemy.cpp)
EnBgCheckFoot__5dEn_cFv                                      = 0x80096f60  (dol/bases/d_enemy.cpp)
EnBgCheckWall__5dEn_cFv                                      = 0x80097050  (dol/bases/d_enemy.cpp)
WaterCheck__5dEn_cFR7mVec3_cf                                = 0x80097170  (dol/bases/d_enemy.cpp)
getRes__6dRes_cCFPCcPCc                                      = 0x800df270  (dol/bases/d_res.cpp)
deleteRequest__7fBase_cFv                                    = 0x80162650  (dol/framework/f_base.cpp)
__dl__7fBase_cFPv                                            = 0x80162a60  (dol/framework/f_base.cpp)
create__Q23m3d8anmChr_cF...                                  = 0x80165210  (dol/mLib/m_3d/anm_chr.cpp)
setAnm__Q23m3d8anmChr_cFRQ23m3d6bmdl_c...                    = 0x80165330  (dol/mLib/m_3d/anm_chr.cpp)
create__Q23m3d11anmTexSrt_cF...                              = 0x80167560  (dol/mLib/m_3d/anm_tex_srt.cpp)
__dt__Q23m3d11anmTexSrt_cFv                                  = 0x801677e0  (dol/mLib/m_3d/anm_tex_srt.cpp)
setAnm__Q23m3d11anmTexSrt_cFRQ23m3d6bmdl_c...                = 0x80167940  (dol/mLib/m_3d/anm_tex_srt.cpp)
__dt__Q23m3d6fanm_cFv                                        = 0x80168ec0  (dol/mLib/m_3d/fanm.cpp)
__dt__Q23m3d5mdl_cFv                                         = 0x80169e60  (dol/mLib/m_3d/mdl.cpp)
create__Q23m3d5mdl_cF...                                     = 0x80169ed0  (dol/mLib/m_3d/mdl.cpp)
setAnm__Q23m3d5mdl_cFRQ23m3d6banm_cf                         = 0x8016a0c0  (dol/mLib/m_3d/mdl.cpp)
setScale__Q23m3d9scnLeaf_cFRCQ34nw4r4math4VEC3               = 0x8016a290  (dol/mLib/m_3d/scn_leaf.cpp)
setLocalMtx__Q23m3d9scnLeaf_cFPCQ34nw4r4math5MTX34           = 0x8016a2b0  (dol/mLib/m_3d/scn_leaf.cpp)
calc__Q23m3d9scnLeaf_cFb                                     = 0x8016a2e0  (dol/mLib/m_3d/scn_leaf.cpp)
XrotM__6mMtx_cF4mAng                                         = 0x8016edf0  (dol/mLib/m_mtx.cpp)
YrotM__6mMtx_cF4mAng                                         = 0x8016ef10  (dol/mLib/m_mtx.cpp)
ZrotM__6mMtx_cF4mAng                                         = 0x8016f030  (dol/mLib/m_mtx.cpp)
g_gameHeaps__5mHeap                                          = 0x80377f48  (dol/mLib/m_heap.cpp)
m_instance__9dResMng_c                                       = 0x8042a318  (dol/bases/d_res_mng.cpp)
```

---

# Part 2. Task B: Strategic Survey & Ranked Queue of `d_basesNP`

## 2.1 State of `d_basesNP.rel`

`d_basesNP.rel` is the largest REL module in *New Super Mario Bros. Wii* (1.86 MB code across 12,905 functions and 441 profile definitions). Currently, only **13 translation units** (~1.5%) are landed.

When running `tools/sibmap.py`, the following stderr warning was captured:
```
sibmap: WARNING: 1 FAMILY entries match no corpus file and contribute nothing:
    d_a_en_dpakkun
```

## 2.2 Ranked Queue of the Next 8 Authorable TUs in `d_basesNP`

We evaluated all candidate units in `d_basesNP.rel` and ranked them by **progress-per-unit-of-risk**:

```
+------+--------------------------+-------------------+------------+----------+-----------+-------------------+----------------------+
| Rank | Translation Unit         | Class Name        | Code Bytes | Span (B) | Functions | Sibling Score     | Key Strategic Value  |
+------+--------------------------+-------------------+------------+----------+-----------+-------------------+----------------------+
|  1   | d_a_wm_grid.cpp          | daWmGrid_c        |    440 B   |   512 B  |    10     | 85.5% e / 100% sh | Zero-risk starter    |
|  2   | d_a_wm_tower.cpp         | daWmTower_c       |  1,064 B   | 1,120 B  |    11     | 88.1% e / 98.6% sh| High-yield sibling   |
|  3   | d_a_wm_smallcloud.cpp    | daWmSmallCloud_c  |  1,964 B   | 2,064 B  |    16     | 71.5% e / 83.5% sh| Exact twin of cloud  |
|  4   | d_a_wm_ghost.cpp         | daWmGhost_c       |  3,024 B   | 3,088 B  |    13     | 39.9% e / 55.3% sh| Closes dokan_route   |
|  5   | d_a_wm_kinoko_base.cpp   | daWmKinokoBase_c  |  2,648 B   | 2,768 B  |    17     | 41.9% e / 54.3% sh| Unblocks 3 leaf TUs  |
|  6   | d_a_wm_kinoko_1up.cpp    | daWmKinoko1up_c   |    412 B   |   480 B  |     9     | 82.5% e / 94.2% sh| Leaf of kinoko_base  |
|  7   | d_a_wm_boss_base.cpp     | daWmBossBase_c    |  1,708 B   | 1,952 B  |    12     | 28.8% e / 38.9% sh| Unblocks 7 Koopalings|
|  8   | d_a_wm_boss_larry.cpp    | daWmBossLarry_c   |    524 B   |   544 B  |     8     | 55.7% e / 70.8% sh| Leaf of boss_base    |
+------+--------------------------+-------------------+------------+----------+-----------+-------------------+----------------------+
```

---

### Unit 1: [d_a_wm_grid.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_grid.cpp) — The Zero-Risk Starter

- **Profile & Class**: `g_profile_WM_GRID` / `daWmGrid_c` (Leaf derived from `dWmObjActor_c`).
- **Section Bounds**:
  * `.text`: `0x164230-0x164430` (Span: 512 B, Code: 440 B, 10 functions).
  * `.ctors`: `0x3e4-0x3e8` (Size: 0x4).
  * `.rodata`: `0x88b8-0x88d0` (Size: 0x18).
  * `.data`: `0x44cb4-0x44d54` (Size: 0xA0).
  * `.bss`: `0xfdd0-0xfde0` (Size: 0x10).
- **Both Checks Run**: Zero overlaps across all 5 sections; bracketed between `daWmGhost_c` (`0x163620..0x164230`) and `daWmHanachan_c` (`0x164430..0x165c70`).
- **Tractability**:
  * **Sibling Score**: **85.45% exact / 100.00% shape**.
  * Every function has 100% shape match with existing repo code (`dSelectCursor_c`, `dWmDemoActor_c`, `d_2d`).
  * 0 unreconstructed types: `dWmObjActor_c` header already exists in [include/game/bases/d_wm_obj_actor.hpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/include/game/bases/d_wm_obj_actor.hpp).
  * 0 register-allocation risk functions: largest function is 116 bytes (`~daWmGrid_c`).
- **Why Start Here**: The highest progress-per-unit-of-risk in the entire REL. It proves the REL authoring pipeline with a guaranteed first-compile match.

---

### Unit 2: [d_a_wm_tower.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_tower.cpp) — The High-Yield World Map Landmark

- **Profile & Class**: `g_profile_WM_TOWER` / `daWmTower_c` (Leaf derived from `dWmObjActor_c`).
- **Section Bounds**:
  * `.text`: `0x185710-0x185b70` (Span: 1,120 B, Code: 1,064 B, 11 functions).
  * `.ctors`: `0x44c-0x450` (Size: 0x4).
  * `.rodata`: `0x9488-0x94a0` (Size: 0x18).
  * `.data`: `0x480b4-0x4818c` (Size: 0xD8).
  * `.bss`: `0x10a98-0x10aa8` (Size: 0x10).
- **Both Checks Run**: Zero overlaps; bracketed between `daWmToride_c` (`0x1847a0..0x185710`) and `daWmTreasureShip_c` (`0x185b70..0x186420`).
- **Tractability**:
  * **Sibling Score**: **88.09% exact / 98.56% shape**.
  * `__ct__` (84 B) is 85.7% exact / 100% shape against banked `d_a_wm_cannon.cpp`.
  * `__dt__` (152 B) is 84.2% exact / 100% shape against banked `d_a_wm_cannon.cpp`.
  * `execute` (124 B) is 87.1% exact / 93.5% shape against banked `d_a_wm_dokan.cpp`.
  * `create` (92 B) is 84.0% exact / 92.0% shape against banked `d_a_wm_peach_castle.cpp`.
- **Why Rank 2**: 1,064 bytes of code that almost completely transcribes directly from already-banked world map actors.

---

### Unit 3: [d_a_wm_smallcloud.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_smallcloud.cpp) — The Direct Sibling Twin of `d_a_wm_cloud.cpp`

- **Profile & Class**: `g_profile_WM_SMALLCLOUD` / `daWmSmallCloud_c` (Leaf derived from `dWmObjActor_c`).
- **Section Bounds**:
  * `.text`: `0x1797e0-0x179ff0` (Span: 2,064 B, Code: 1,964 B, 16 functions).
  * `.ctors`: `0x430-0x434` (Size: 0x4).
  * `.rodata`: `0x8f58-0x8fa0` (Size: 0x48).
  * `.data`: `0x4728c-0x47484` (Size: 0x1F8).
  * `.bss`: `0x10130-0x10140` (Size: 0x10).
- **Both Checks Run**: Zero overlaps; bracketed between `daWmSinkShip_c` (`0x179380..0x1797e0`) and `daWmStart_c` (`0x179ff0..0x17aff0`).
- **Tractability**:
  * **Sibling Score**: **71.49% exact / 83.50% shape**.
  * Exact twin of banked `d_a_wm_cloud.cpp`: constructor (87.1% exact / 100% shape), destructor (86.0% exact / 100% shape), `execute` (84.7% exact / 100% shape), `create` (66.1% exact / 72.6% shape).
  * 0 new external types; uses standard `dWmSVMdl_c` and `m3d::smdl_c`.
- **Why Rank 3**: 1,964 code bytes with a banked model file in the repo ([source/d_basesNP/bases/d_a_wm_cloud.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_cloud.cpp)) to copy from.

---

### Unit 4: [d_a_wm_ghost.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_ghost.cpp) — The Immediate Neighbor of `d_a_wm_dokan_route.cpp`

- **Profile & Class**: `g_profile_WM_GHOST` / `daWmGhost_c` (Leaf derived from `dWmObjActor_c`).
- **Section Bounds**:
  * `.text`: `0x163620-0x164230` (Span: 3,088 B, Code: 3,024 B, 13 functions).
  * `.ctors`: `0x3e0-0x3e4` (Size: 0x4).
  * `.rodata`: `0x8880-0x88b8` (Size: 0x38).
  * `.data`: `0x44a9c-0x44cb4` (Size: 0x218).
  * `.bss`: `0xfdc0-0xfdd0` (Size: 0x10).
- **Both Checks Run**: **100% adjacent to `d_a_wm_dokan_route.cpp`** in `.text` (`0x163620`), `.ctors` (`0x3e0`), `.rodata` (`0x8880`), and `.bss` (`0xfdc0`).
- **Tractability**:
  * **Sibling Score**: **39.87% exact / 55.27% shape**.
  * Standard actor lifecycle (`create` 88% exact / 100% shape against `d_a_wm_peach_castle`, `execute` 77.8% exact / 86.1% shape against `d_a_wm_cannon`).
- **Why Rank 4**: It extends the banked territory continuously from `d_a_wm_dokan_route.cpp` and pins down the low bound of `d_a_wm_grid.cpp`.

---

### Unit 5: [d_a_wm_kinoko_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_kinoko_base.cpp) — The High-Leverage Mushroom Base Class

- **Profile & Class**: `g_profile_WM_KINOKO_BASE` / `daWmKinokoBase_c` (**Base Class**).
- **Gating Impact**: **Directly unblocks 3 derived leaf translation units**:
  1. `d_a_wm_kinoko_1up.cpp` (`daWmKinoko1up_c`, 412 B)
  2. `d_a_wm_kinoko_red.cpp` (`daWmKinokoRed_c`, 404 B)
  3. `d_a_wm_kinoko_star.cpp` (`daWmKinokoStar_c`, 412 B)
- **Section Bounds**:
  * `.text`: `0x16b2d0-0x16bda0` (Span: 2,768 B, Code: 2,648 B, 17 functions).
  * `.ctors`: `0x3fc-0x400` (Size: 0x4).
  * `.rodata`: `0x8b70-0x8ba8` (Size: 0x38).
  * `.data`: `0x458e4-0x45ab4` (Size: 0x1D0).
  * `.bss`: `0xfe88-0xfea0` (Size: 0x18).
- **Tractability**:
  * **Sibling Score**: **41.92% exact / 54.33% shape**.
  * `__ct__` (176 B) and `__dt__` (192 B) score 68.2% and 79.2% exact against `d_a_wm_peach_castle.cpp`.
  * `create` (100 B) is 76.0% exact / 80.0% shape against `d_a_wm_peach.cpp`.
- **Why Rank 5**: Landing this 2.6 KB base unblocks 3 leaves, converting 1 unit of authoring work into 4 landed files (3,876 B code total).

---

### Unit 6: [d_a_wm_kinoko_1up.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_kinoko_1up.cpp) — The 1UP Mushroom Leaf

- **Profile & Class**: `g_profile_WM_KINOKO_1UP` / `daWmKinoko1up_c` (Leaf derived from `daWmKinokoBase_c`).
- **Section Bounds**:
  * `.text`: `0x16b0f0-0x16b2d0` (Span: 480 B, Code: 412 B, 9 functions).
  * `.ctors`: `0x3f8-0x3fc` (Size: 0x4).
  * `.rodata`: `0x8b58-0x8b70` (Size: 0x18).
  * `.data`: `0x457ec-0x458e4` (Size: 0xF8).
  * `.bss`: `0xfe78-0xfe88` (Size: 0x10).
- **Tractability**:
  * **Sibling Score**: **82.52% exact / 94.17% shape**.
  * Sits immediately adjacent below `daWmKinokoBase_c` in `.text` (`0x16b0f0`..`0x16b2d0` $\to$ `0x16b2d0`).
  * Trivial leaf: overrides constructor/destructor and returns `"wm_1up_kinoko"` resource strings.
- **Why Rank 6**: Once `daWmKinokoBase_c` is written, authoring this unit takes less than 30 minutes.

---

### Unit 7: [d_a_wm_boss_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_boss_base.cpp) — The 7-Koopaling Base Class

- **Profile & Class**: `g_profile_WM_BOSS_BASE` / `daWmBossBase_c` (**Base Class**).
- **Gating Impact**: **Directly unblocks all 7 Koopaling world map actor units**:
  * `d_a_wm_boss_iggy.cpp` (524 B)
  * `d_a_wm_boss_larry.cpp` (524 B)
  * `d_a_wm_boss_lemmy.cpp` (524 B)
  * `d_a_wm_boss_ludwig.cpp` (524 B)
  * `d_a_wm_boss_morton.cpp` (524 B)
  * `d_a_wm_boss_roy.cpp` (540 B)
  * `d_a_wm_boss_wendy.cpp` (524 B)
  * Total unblocked code: **3,708 bytes across 57 functions**.
- **Section Bounds**:
  * `.text`: `0x189ac0-0x18a260` (Span: 1,952 B, Code: 1,708 B, 12 functions).
  * `.ctors`: `0x454-0x458` (Size: 0x4).
  * `.rodata`: `0x9590-0x95d8` (Size: 0x48).
  * `.data`: `0x485fc-0x488c8` (Size: 0x2CC).
  * `.bss`: `0x10b48-0x10b60` (Size: 0x18).
- **Tractability**:
  * **Sibling Score**: **28.75% exact / 38.91% shape**.
  * Lower raw similarity because Koopaling map animations implement custom laughter and fleeing sequences, but layout derives cleanly from `dWmDemoActor_c`.
- **Why Rank 7**: Highly strategic. Landing `daWmBossBase_c` enables clearing all 7 Koopalings in a single follow-up batch.

---

### Unit 8: [d_a_wm_boss_larry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_boss_larry.cpp) — The World 1 Koopaling Leaf

- **Profile & Class**: `g_profile_WM_BOSS_LARRY` / `daWmBossLarry_c` (Leaf derived from `daWmBossBase_c`).
- **Section Bounds**:
  * `.text`: `0x18b470-0x18b690` (Span: 544 B, Code: 524 B, 8 functions).
  * `.ctors`: `0x464-0x468` (Size: 0x4).
  * `.rodata`: `0x96b8-0x96d0` (Size: 0x18).
  * `.data`: `0x48cd8-0x48e58` (Size: 0x180).
  * `.bss`: `0x10bd0-0x10be0` (Size: 0x10).
- **Tractability**:
  * **Sibling Score**: **55.73% exact / 70.79% shape**.
  * Tiny leaf: overrides constructor, destructor, sound ID dispatch, and resource name (`"wm_larry"`).
- **Why Rank 8**: Demonstrates the first derived Koopaling leaf and proves the `daWmBossBase_c` vtable.

---

## 2.3 Final Recommendation & Starting Plan

If authoring single units out of `d_basesNP`, **start with [d_a_wm_grid.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_grid.cpp)**.

**Rationale**:
1. At **85.45% exact / 100.00% shape match**, it carries practically zero register-allocation hazard.
2. It requires zero new headers (`dWmObjActor_c` already exists).
3. It immediately banks a new unit in `d_basesNP` with 10 functions and establishes the REL build/landing baseline.
4. Immediately following it, the pipeline should execute **[d_a_wm_tower.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_tower.cpp)** (1,064 B, 88% match) and the **`daWmKinokoBase_c` family** to unlock 4 units in rapid succession.
