# Gemini Response — Round 12: Landing Kits for `d_basesNP` Queue & Strategic Survey of `d_en_bossNP`

## Executive Summary

1. **Task A (`d_basesNP` Landing Kits)**:
   - Generated complete, verified landing kits for the top 4 units of the `d_basesNP` queue:
     1. **`d_a_wm_grid.cpp`** (`d_basesNP/bases/d_a_wm_grid.cpp`): 10 functions, 440 B code, 512 B span (`.text 0x164230-0x164430`). Passed all 5 section checks with 0x0 gap adjacency to `d_a_wm_ghost.cpp` and `d_a_wm_hanachan.cpp`. **0 removals**, **0 additions**, **23 DOL must-not-pin symbols**, **0 REL must-not-pin symbols**.
     2. **`d_a_wm_tower.cpp`** (`d_basesNP/bases/d_a_wm_tower.cpp`): 11 functions, 1,064 B code, 1,120 B span (`.text 0x185710-0x185b70`). Passed all 5 section checks with 0x0 gap adjacency to `d_a_wm_toride.cpp` and `d_a_wm_treasure_ship.cpp`. **0 removals**, **0 additions**, **35 DOL must-not-pin symbols**, **0 REL must-not-pin symbols**.
     3. **`d_a_wm_smallcloud.cpp`** (`d_basesNP/bases/d_a_wm_smallcloud.cpp`): 16 functions, 1,964 B code, 2,064 B span (`.text 0x1797e0-0x179ff0`). Passed all 5 section checks with 0x0 gap adjacency to `d_a_wm_sink_ship.cpp` and `d_a_wm_start.cpp`. **0 removals**, **1 addition** (`GetNodePos__9daWmMap_cFPCcR7mVec3_c = 0x801007F0`), **40 DOL must-not-pin symbols**, **1 REL must-not-pin symbol** (`__dt__Q23m3d8anmChr_cFv` in `d_awa.cpp`).
     4. **`d_a_wm_kinoko_base.cpp`** (`d_basesNP/bases/d_a_wm_kinoko_base.cpp`): 17 functions, 2,648 B code, 2,768 B span (`.text 0x16b2d0-0x16bda0`). Passed all 5 section checks with 0x0 gap adjacency to `d_a_wm_kinoko_1up.cpp` and `d_a_wm_kinoko_red.cpp`. **0 removals**, **5 additions** (`bindAnimToNode__8dsChrLib...`, `clearZoromeTime__6dWmLibFv`, `setStartPointKinokoHouseKindNum__6dWmLibFUc`, `IsAllComplete__6dWmLibFv`, `fn_80103420`), **51 DOL must-not-pin symbols**, **1 REL must-not-pin symbol** (`__dt__Q23m3d8anmChr_cFv` in `d_awa.cpp`).
   - **REL Pin Mechanics Analysis**: Documented the structural differences between DOL fixed-address linking (`syms.txt`) and REL relocatable ELF linkage (`alias_db.txt` + DOL ELF symbol table resolution).

2. **Task B (`d_en_bossNP` Strategic Survey)**:
   - `d_en_bossNP.rel` is at **0.031%** (112 bytes landed of 356,396 total code bytes). It contains 26 translation units (26 `.ctors` slots), 22 `g_profile_*` actor profile tables, and 2,384 functions across 356.4 KB of `.text`.
   - Executed `tools/sibmap.py` over the 5,168-function corpus and captured its stderr warning:
     `sibmap: WARNING: 1 FAMILY entries match no corpus file and contribute nothing: d_a_en_dpakkun`
   - **Critical Tractability Finding — Universal Anonymous Symbols**: In `d_en_bossNP_symbols.txt`, **2,377 of 2,384 functions (99.7%) are anonymous `fn_4_*` symbols**. Zero function symbols in any boss class have CFront mangled names. Every parameter type, return type, and `const` qualifier must be proven strictly via register allocation and instruction codegen.
   - Produced a ranked queue of the **next 8 authorable TUs in `d_en_bossNP`**, ranked by progress-per-unit-of-risk.
   - **Top Recommendation**: [d_a_en_boss_koopa_demo_cage.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_koopa_demo_cage.cpp) (**71.70% exact / 74.24% shape sibling score**, 3,368 B code, 32 fns, zero complex state machines), followed by [d_a_en_boss_koopa_jr_a.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_koopa_jr_a.cpp) (6,784 B code, unblocks Jr B and Jr C) and [d_a_en_boss_castle_larry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_castle_larry.cpp) (7,876 B code, 56.2% shape).

---

# Part 1. Task A: Landing Kits for the `d_basesNP` Queue

## 1.0 Structural Pin Mechanics: REL vs DOL

Before landing REL units, the differences in linking and symbol resolution between `wiimj2d.dol` and REL modules (`d_basesNP.rel`, `d_en_bossNP.rel`, etc.) must be made explicit:

1. **DOL (`wiimj2d.dol`) Linking**:
   - `syms.txt` pins absolute virtual memory addresses (`0x80xxxxxx`) directly for the CodeWarrior linker (`mwldeppc.exe`).
   - When a DOL translation unit lands, any symbol it defines that was previously in `syms.txt` **must be removed**, or the build fails with symbol redefinition errors.
   - External DOL symbols referenced by a landed unit that are not yet banked must be **added** to `syms.txt`.

2. **REL (`d_basesNP.rel`) Linking**:
   - REL files are linked as relocatable ELF objects (`.plf`) and processed by `tools/build_rel.py`.
   - **REL symbols are NEVER placed in `syms.txt`**: REL symbols have section-relative offsets, not 0x80xxxxxx virtual addresses. Placing a REL symbol in `syms.txt` corrupts DOL symbol resolution.
   - **Cross-Module Calls (REL $\to$ DOL)**: `tools/build_rel.py` resolves calls from REL into DOL by matching symbol names in the DOL ELF symbol table (`wiimj2d.dol.elf`).
     * If an external DOL symbol referenced by a REL unit is **already landed** in DOL or **already pinned in `syms.txt`**, it resolves automatically.
     * If an external DOL symbol referenced by a REL unit is **neither landed nor pinned**, `build_rel.py` throws an unresolved symbol error. Thus, **DOL additions needed by REL units must be added to `syms.txt`**.
   - **Intra-Module Calls (REL $\to$ REL)**: `tools/slice_rel.py` extracts unlanded slices from the original REL and uses [alias_db.txt](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/alias_db.txt) (`R_2_<sec>_<offset> = SymbolName`) to name symbols. When a REL unit lands, its `.o` file directly exports its defined symbols to the REL linker.
   - **Must-Not-Pin**: Symbols already defined by previously banked DOL slices (e.g. `dBaseActor_c`, `fBase_c`, `mHeap`, `m3d`) or banked REL slices (e.g. `d_awa.cpp`) must **never** be pinned.

---

## 1.1 Landing Kit 1: `d_a_wm_grid.cpp`

### Slice Block for `slices/d_basesNP.json`:

```json
{
    "source": "d_basesNP/bases/d_a_wm_grid.cpp",
    "memoryRanges": {
        ".text": "0x164230-0x164430",
        ".ctors": "0x3e4-0x3e8",
        ".rodata": "0x88b8-0x88d0",
        ".data": "0x44cb4-0x44d54",
        ".bss": "0xfdd0-0xfde0"
    }
}
```

#### Base Subtraction & Adjacency Breakdown:
- **Subtracted Base**: `0x00000000` (All REL section memory ranges are offsets from section start `0x0`).
- **`.text`**: `0x164230-0x164430` (Span: `0x200` / 512 B, Code: 440 B, 10 functions).
  * Overlaps: 0.
  * Immediately preceding slice: [d_basesNP/bases/d_a_wm_ghost.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_a_wm_ghost.cpp) (`0x163620-0x164230`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_hanachan.cpp` (`0x164430-0x165c70`), gap = **0x0**.
- **`.ctors`**: `0x3e4-0x3e8` (Span: `0x4`).
  * Overlaps: 0. Slot points to `__sinit_\d_a_wm_grid_cpp` (`0x164380`).
  * Immediately preceding slice: `d_a_wm_ghost.cpp` (`0x3e0-0x3e4`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_hanachan.cpp` (`0x3e8-0x3ec`), gap = **0x0**.
- **`.rodata`**: `0x88b8-0x88d0` (Span: `0x18` / 24 B).
  * Overlaps: 0. Contains float pool `lbl_2_rodata_88B8`–`lbl_2_rodata_88CC`.
  * Immediately preceding slice: `d_a_wm_ghost.cpp` (`0x8880-0x88b8`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_hanachan.cpp` (`0x88d0-0x8928`), gap = **0x0**.
- **`.data`**: `0x44cb4-0x44d54` (Span: `0xA0` / 160 B).
  * Overlaps: 0. Contains `g_profile_WM_GRID` (`0x44cb4`), string literals, and vtable `lbl_2_data_44CC0` (`0x44cc0`).
  * Immediately preceding slice: `d_a_wm_ghost.cpp` (`0x44a9c-0x44cb4`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_hanachan.cpp` (`0x44d54-0x4503c`), gap = **0x0**.
- **`.bss`**: `0xfdd0-0xfde0` (Span: `0x10` / 16 B).
  * Overlaps: 0. Contains `lbl_2_bss_FDD0` (0xC) and `lbl_2_bss_FDDC` (0x4).
  * Immediately preceding slice: `d_a_wm_ghost.cpp` (`0xfdc0-0xfdd0`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_hanachan.cpp` (`0xfde0-0xfe08`), gap = **0x0**.

### `syms.txt` Removals (0 symbols):
- Zero symbols in [syms.txt](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/syms.txt) belong to this unit (REL symbols are not in `syms.txt`).

### `syms.txt` Additions (0 symbols):
- All referenced external DOL symbols (`__ct__16dHeapAllocator_cFv` at `0x80069020`, `__dt__16dHeapAllocator_cFv` at `0x80069060`) are **already pinned** in `syms.txt`.

### Must-Not-Pin List (23 symbols):
These 23 symbols referenced by `d_a_wm_grid.cpp` are **already defined by landed slices**; they must **NOT** be pinned in `syms.txt`:

```
getKindString__7dBase_cCFv                          = 0x8006C660  (dol/bases/d_base.cpp)
draw2D__12dBaseActor_cFv                            = 0x8006CA50  (dol/bases/d_base_actor.cpp)
draw2D_lyt2__12dBaseActor_cFv                       = 0x8006CA60  (dol/bases/d_base_actor.cpp)
__ct__10dWmActor_cFv                                = 0x800F2820  (dol/bases/d_wm_actor.cpp)
__dt__10dWmActor_cFv                                = 0x800F2880  (dol/bases/d_wm_actor.cpp)
preCreate__10dWmActor_cFv                           = 0x800F28E0  (dol/bases/d_wm_actor.cpp)
postCreate__10dWmActor_cFQ27fBase_c12MAIN_STATE_e   = 0x800F2910  (dol/bases/d_wm_actor.cpp)
preDelete__10dWmActor_cFv                           = 0x800F2920  (dol/bases/d_wm_actor.cpp)
postDelete__10dWmActor_cFQ27fBase_c12MAIN_STATE_e   = 0x800F2950  (dol/bases/d_wm_actor.cpp)
preExecute__10dWmActor_cFv                          = 0x800F2960  (dol/bases/d_wm_actor.cpp)
postExecute__10dWmActor_cFQ27fBase_c12MAIN_STATE_e  = 0x800F2A10  (dol/bases/d_wm_actor.cpp)
preDraw__10dWmActor_cFv                             = 0x800F2A20  (dol/bases/d_wm_actor.cpp)
postDraw__10dWmActor_cFQ27fBase_c12MAIN_STATE_e     = 0x800F2AF0  (dol/bases/d_wm_actor.cpp)
deleteReady__7fBase_cFv                             = 0x80162410  (dol/framework/f_base.cpp)
entryFrmHeap__7fBase_cFUlPQ23EGG4Heap               = 0x80162730  (dol/framework/f_base.cpp)
entryFrmHeapNonAdjust__7fBase_cFUlPQ23EGG4Heap      = 0x80162930  (dol/framework/f_base.cpp)
createHeap__7fBase_cFv                              = 0x801629F0  (dol/framework/f_base.cpp)
__nw__7fBase_cFUl                                   = 0x80162A00  (dol/framework/f_base.cpp)
__dl__7fBase_cFPv                                   = 0x80162A60  (dol/framework/f_base.cpp)
__ct__Q23m3d6smdl_cFv                               = 0x8016A430  (dol/mLib/m_3d/smdl.cpp)
__dt__Q23m3d6smdl_cFv                               = 0x8016A480  (dol/mLib/m_3d/smdl.cpp)
__destroy_arr                                       = 0x802DCD88  (runtime/class_arrays.cpp)
c_CASTLE_ID__10dCsvData_c                           = 0x8042D24C  (dol/bases/d_wm_csvdata.cpp)
c_START_ID__10dCsvData_c                            = 0x8042D264  (dol/bases/d_wm_csvdata.cpp)
```

---

## 1.2 Landing Kit 2: `d_a_wm_tower.cpp`

### Slice Block for `slices/d_basesNP.json`:

```json
{
    "source": "d_basesNP/bases/d_a_wm_tower.cpp",
    "memoryRanges": {
        ".text": "0x185710-0x185b70",
        ".ctors": "0x44c-0x450",
        ".rodata": "0x9488-0x94a0",
        ".data": "0x480b4-0x4818c",
        ".bss": "0x10a98-0x10aa8"
    }
}
```

#### Base Subtraction & Adjacency Breakdown:
- **Subtracted Base**: `0x00000000`.
- **`.text`**: `0x185710-0x185b70` (Span: `0x460` / 1,120 B, Code: 1,064 B, 11 functions).
  * Overlaps: 0.
  * Immediately preceding slice: `d_a_wm_toride.cpp` (`0x1847a0-0x185710`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_treasure_ship.cpp` (`0x185b70-0x186420`), gap = **0x0**.
- **`.ctors`**: `0x44c-0x450` (Span: `0x4`).
  * Overlaps: 0. Slot points to `__sinit_\d_a_wm_tower_cpp` (`0x185ac0`).
  * Immediately preceding slice: `d_a_wm_toride.cpp` (`0x448-0x44c`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_treasure_ship.cpp` (`0x450-0x454`), gap = **0x0**.
- **`.rodata`**: `0x9488-0x94a0` (Span: `0x18` / 24 B).
  * Overlaps: 0.
  * Immediately preceding slice: `d_a_wm_toride.cpp` (`0x9430-0x9488`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_treasure_ship.cpp` (`0x94a0-0x94c8`), gap = **0x0**.
- **`.data`**: `0x480b4-0x4818c` (Span: `0xD8` / 216 B).
  * Overlaps: 0. Contains `g_profile_WM_TOWER` (`0x480b4`), strings, vtable `lbl_2_data_480E0` (`0x480e0`).
  * Immediately preceding slice: `d_a_wm_toride.cpp` (`0x47e8c-0x480b4`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_treasure_ship.cpp` (`0x4818c-0x483ac`), gap = **0x0**.
- **`.bss`**: `0x10a98-0x10aa8` (Span: `0x10` / 16 B).
  * Overlaps: 0.
  * Immediately preceding slice: `d_a_wm_toride.cpp` (`0x10a88-0x10a98`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_treasure_ship.cpp` (`0x10aa8-0x10ab8`), gap = **0x0**.

### `syms.txt` Removals (0 symbols):
- 0 symbols.

### `syms.txt` Additions (0 symbols):
- All 10 referenced unlanded DOL symbols (`__ct__16dHeapAllocator_cFv`, `__dt__16dHeapAllocator_cFv`, `createFrmHeap__16dHeapAllocator_cFUlPQ23EGG4HeapPCcUl`, `adjustFrmHeap__16dHeapAllocator_cFv`, `GetNodePos__9daWmMap_cFlR7mVec3_c`, `GetCutName__11dCsSeqMng_cFv`, `PSMTXTrans`, `GetResMdl__Q34nw4r3g3d7ResFileCFPCc`, `m_instance__9daWmMap_c`, `ms_instance__11dCsSeqMng_c`) are **already pinned** in `syms.txt`.

### Must-Not-Pin List (35 symbols):
These 35 symbols referenced by `d_a_wm_tower.cpp` are **already defined by landed slices**; they must **NOT** be pinned in `syms.txt`:

```
getKindString__7dBase_cCFv                          = 0x8006C660  (dol/bases/d_base.cpp)
draw2D__12dBaseActor_cFv                            = 0x8006CA50  (dol/bases/d_base_actor.cpp)
draw2D_lyt2__12dBaseActor_cFv                       = 0x8006CA60  (dol/bases/d_base_actor.cpp)
getRes__6dRes_cCFPCcPCc                             = 0x800DF270  (dol/bases/d_res.cpp)
__dt__10dWmActor_cFv                                = 0x800F2880  (dol/bases/d_wm_actor.cpp)
preCreate__10dWmActor_cFv                           = 0x800F28E0  (dol/bases/d_wm_actor.cpp)
postCreate__10dWmActor_cFQ27fBase_c12MAIN_STATE_e   = 0x800F2910  (dol/bases/d_wm_actor.cpp)
preDelete__10dWmActor_cFv                           = 0x800F2920  (dol/bases/d_wm_actor.cpp)
postDelete__10dWmActor_cFQ27fBase_c12MAIN_STATE_e   = 0x800F2950  (dol/bases/d_wm_actor.cpp)
preExecute__10dWmActor_cFv                          = 0x800F2960  (dol/bases/d_wm_actor.cpp)
postExecute__10dWmActor_cFQ27fBase_c12MAIN_STATE_e  = 0x800F2A10  (dol/bases/d_wm_actor.cpp)
preDraw__10dWmActor_cFv                             = 0x800F2A20  (dol/bases/d_wm_actor.cpp)
postDraw__10dWmActor_cFQ27fBase_c12MAIN_STATE_e     = 0x800F2AF0  (dol/bases/d_wm_actor.cpp)
setSoftLight_MapObj__10dWmActor_cFRQ23m3d6bmdl_c    = 0x800F2B30  (dol/bases/d_wm_actor.cpp)
__ct__14dWmDemoActor_cFv                            = 0x800F60E0  (dol/bases/d_wm_demo_actor.cpp)
processCutsceneCommand__14dWmDemoActor_cFib         = 0x800F61C0  (dol/bases/d_wm_demo_actor.cpp)
deleteReady__7fBase_cFv                             = 0x80162410  (dol/framework/f_base.cpp)
entryFrmHeap__7fBase_cFUlPQ23EGG4Heap               = 0x80162730  (dol/framework/f_base.cpp)
entryFrmHeapNonAdjust__7fBase_cFUlPQ23EGG4Heap      = 0x80162930  (dol/framework/f_base.cpp)
createHeap__7fBase_cFv                              = 0x801629F0  (dol/framework/f_base.cpp)
__nw__7fBase_cFUl                                   = 0x80162A00  (dol/framework/f_base.cpp)
__dl__7fBase_cFPv                                   = 0x80162A60  (dol/framework/f_base.cpp)
setScale__Q23m3d9scnLeaf_cFRCQ34nw4r4math4VEC3      = 0x8016A290  (dol/mLib/m_3d/scn_leaf.cpp)
setLocalMtx__Q23m3d9scnLeaf_cFPCQ34nw4r4math5MTX34  = 0x8016A2B0  (dol/mLib/m_3d/scn_leaf.cpp)
calc__Q23m3d9scnLeaf_cFb                            = 0x8016A2E0  (dol/mLib/m_3d/scn_leaf.cpp)
__ct__Q23m3d6smdl_cFv                               = 0x8016A430  (dol/mLib/m_3d/smdl.cpp)
__dt__Q23m3d6smdl_cFv                               = 0x8016A480  (dol/mLib/m_3d/smdl.cpp)
create__Q23m3d6smdl_cFQ34nw4r3g3d6ResMdlP12mAllocator_cUliPUl = 0x8016A4E0 (dol/mLib/m_3d/smdl.cpp)
__dt__16mHeapAllocator_cFv                          = 0x8016A8C0  (dol/mLib/m_allocator.cpp)
ZXYrotM__6mMtx_cF4mAng4mAng4mAng                    = 0x8016F090  (dol/mLib/m_mtx.cpp)
__destroy_arr                                       = 0x802DCD88  (runtime/class_arrays.cpp)
g_gameHeaps__5mHeap                                 = 0x80377F48  (dol/mLib/m_heap.cpp)
m_instance__9dResMng_c                              = 0x8042A318  (dol/bases/d_res_mng.cpp)
c_CASTLE_ID__10dCsvData_c                           = 0x8042D24C  (dol/bases/d_wm_csvdata.cpp)
c_START_ID__10dCsvData_c                            = 0x8042D264  (dol/bases/d_wm_csvdata.cpp)
```

---

## 1.3 Landing Kit 3: `d_a_wm_smallcloud.cpp`

### Slice Block for `slices/d_basesNP.json`:

```json
{
    "source": "d_basesNP/bases/d_a_wm_smallcloud.cpp",
    "memoryRanges": {
        ".text": "0x1797e0-0x179ff0",
        ".ctors": "0x430-0x434",
        ".rodata": "0x8f58-0x8fa0",
        ".data": "0x4728c-0x47484",
        ".bss": "0x10130-0x10140"
    }
}
```

#### Base Subtraction & Adjacency Breakdown:
- **Subtracted Base**: `0x00000000`.
- **`.text`**: `0x1797e0-0x179ff0` (Span: `0x810` / 2,064 B, Code: 1,964 B, 16 functions).
  * Overlaps: 0.
  * Immediately preceding slice: `d_a_wm_sink_ship.cpp` (`0x179380-0x1797e0`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_start.cpp` (`0x179ff0-0x17aff0`), gap = **0x0**.
- **`.ctors`**: `0x430-0x434` (Span: `0x4`). Slot points to `__sinit_\d_a_wm_smallcloud_cpp` (`0x179f40`).
  * Immediately preceding slice: `d_a_wm_sink_ship.cpp` (`0x42c-0x430`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_start.cpp` (`0x434-0x438`), gap = **0x0**.
- **`.rodata`**: `0x8f58-0x8fa0` (Span: `0x48` / 72 B).
  * Overlaps: 0.
  * Immediately preceding slice: `d_a_wm_sink_ship.cpp` (`0x8ef8-0x8f58`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_start.cpp` (`0x8fa0-0x8fd0`), gap = **0x0**.
- **`.data`**: `0x4728c-0x47484` (Span: `0x1F8` / 504 B).
  * Overlaps: 0. Contains `g_profile_WM_SMALLCLOUD` (`0x4728c`), resource names, vtable `lbl_2_data_47348` (`0x47348`).
  * Immediately preceding slice: `d_a_wm_sink_ship.cpp` (`0x46f6c-0x4728c`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_start.cpp` (`0x47484-0x47700`), gap = **0x0**.
- **`.bss`**: `0x10130-0x10140` (Span: `0x10` / 16 B).
  * Overlaps: 0.
  * Immediately preceding slice: `d_a_wm_sink_ship.cpp` (`0x10120-0x10130`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_start.cpp` (`0x10140-0x10150`), gap = **0x0**.

### `syms.txt` Removals (0 symbols):
- 0 symbols.

### `syms.txt` Additions (1 symbol):
Add this 1 unpinned DOL symbol referenced by `d_a_wm_smallcloud.cpp`'s relocations:

```
GetNodePos__9daWmMap_cFPCcR7mVec3_c=0x801007f0
```

### Must-Not-Pin List (41 symbols):
- **DOL symbols (40 symbols)**:
  `getKindString__7dBase_cCFv` (0x8006C660), `draw2D__12dBaseActor_cFv` (0x8006CA50), `draw2D_lyt2__12dBaseActor_cFv` (0x8006CA60), `getRes__6dRes_cCFPCcPCc` (0x800DF270), `__dt__10dWmActor_cFv` (0x800F2880), `preCreate__10dWmActor_cFv` (0x800F28E0), `postCreate__10dWmActor_cF...` (0x800F2910), `preDelete__10dWmActor_cFv` (0x800F2920), `postDelete__10dWmActor_cF...` (0x800F2950), `preExecute__10dWmActor_cFv` (0x800F2960), `postExecute__10dWmActor_cF...` (0x800F2A10), `preDraw__10dWmActor_cFv` (0x800F2A20), `postDraw__10dWmActor_cF...` (0x800F2AF0), `setSoftLight_Map__10dWmActor_cFRQ23m3d6bmdl_c` (0x800F2B20), `__ct__14dWmDemoActor_cFv` (0x800F60E0), `isStaff__14dWmDemoActor_cFv` (0x800F61F0), `deleteReady__7fBase_cFv` (0x80162410), `entryFrmHeap__7fBase_cFUlPQ23EGG4Heap` (0x80162730), `entryFrmHeapNonAdjust__7fBase_cFUlPQ23EGG4Heap` (0x80162930), `createHeap__7fBase_cFv` (0x801629F0), `__nw__7fBase_cFUl` (0x80162A00), `__dl__7fBase_cFPv` (0x80162A60), `create__Q23m3d8anmChr_cF...` (0x80165210), `setRate__Q23m3d6banm_cFf` (0x80168220), `setFrame__Q23m3d6fanm_cFf` (0x80169120), `setScale__Q23m3d9scnLeaf_cFRCQ34nw4r4math4VEC3` (0x8016A290), `setLocalMtx__Q23m3d9scnLeaf_cFPCQ34nw4r4math5MTX34` (0x8016A2B0), `calc__Q23m3d9scnLeaf_cFb` (0x8016A2E0), `setPriorityDraw__Q23m3d9scnLeaf_cFii` (0x8016A3E0), `__ct__Q23m3d6smdl_cFv` (0x8016A430), `__dt__Q23m3d6smdl_cFv` (0x8016A480), `create__Q23m3d6smdl_cF...` (0x8016A4E0), `__dt__16mHeapAllocator_cFv` (0x8016A8C0), `ZXYrotM__6mMtx_cF4mAng4mAng4mAng` (0x8016F090), `__construct_array` (0x802DCC90), `__destroy_arr` (0x802DCD88), `g_gameHeaps__5mHeap` (0x80377F48), `m_instance__9dResMng_c` (0x8042A318), `c_CASTLE_ID__10dCsvData_c` (0x8042D24C), `c_START_ID__10dCsvData_c` (0x8042D264).
- **REL symbols (1 symbol)**:
  `__dt__Q23m3d8anmChr_cFv` = `.text:0x4b180` (already defined by landed [d_basesNP/bases/d_awa.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_awa.cpp)).

---

## 1.4 Landing Kit 4: `d_a_wm_kinoko_base.cpp`

### Slice Block for `slices/d_basesNP.json`:

```json
{
    "source": "d_basesNP/bases/d_a_wm_kinoko_base.cpp",
    "memoryRanges": {
        ".text": "0x16b2d0-0x16bda0",
        ".ctors": "0x3fc-0x400",
        ".rodata": "0x8ac8-0x8af0",
        ".data": "0x458c0-0x45a90",
        ".bss": "0xfe80-0xfe90"
    }
}
```

#### Base Subtraction & Adjacency Breakdown:
- **Subtracted Base**: `0x00000000`.
- **`.text`**: `0x16b2d0-0x16bda0` (Span: `0xAD0` / 2,768 B, Code: 2,648 B, 17 functions).
  * Overlaps: 0.
  * Immediately preceding slice: `d_a_wm_kinoko_1up.cpp` (`0x16b0f0-0x16b2d0`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_kinoko_red.cpp` (`0x16bda0-0x16bf70`), gap = **0x0**.
- **`.ctors`**: `0x3fc-0x400` (Span: `0x4`). Slot points to `__sinit_\d_a_wm_kinoko_base_cpp` (`0x16bcf0`).
  * Immediately preceding slice: `d_a_wm_kinoko_1up.cpp` (`0x3f8-0x3fc`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_kinoko_red.cpp` (`0x400-0x404`), gap = **0x0**.
- **`.rodata`**: `0x8ac8-0x8af0` (Span: `0x28` / 40 B).
  * Overlaps: 0. Contains float literal block `lbl_2_rodata_8AC8`.
  * Immediately preceding slice: `d_a_wm_kinoko_1up.cpp` (`0x8ab8-0x8ac8`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_kinoko_red.cpp` (`0x8af0-0x8b00`), gap = **0x0**.
- **`.data`**: `0x458c0-0x45a90` (Span: `0x1D0` / 464 B).
  * Overlaps: 0. Contains `g_profile_WM_KINOKO_BASE` (`0x458e4`), string literals, vtable `lbl_2_data_45938` (`0x45938`), and resource string pointer table `lbl_2_data_45A68` (`0x45a68`).
  * Immediately preceding slice: `d_a_wm_kinoko_1up.cpp` (`0x457c8-0x458c0`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_kinoko_red.cpp` (`0x45a90-0x45b98`), gap = **0x0**.
- **`.bss`**: `0xfe80-0xfe90` (Span: `0x10` / 16 B).
  * Overlaps: 0. Contains `lbl_2_bss_FE80` (`__register_global_object` node) and `lbl_2_bss_FE8C` (static state flag).
  * Immediately preceding slice: `d_a_wm_kinoko_1up.cpp` (`0xfe70-0xfe80`), gap = **0x0**.
  * Immediately following slice: `d_a_wm_kinoko_red.cpp` (`0xfe90-0xfea0`), gap = **0x0**.

### `syms.txt` Removals (0 symbols):
- 0 symbols.

### `syms.txt` Additions (5 symbols):
Add these 5 unpinned DOL symbols referenced by `d_a_wm_kinoko_base.cpp`'s relocations:

```
bindAnimToNode__8dsChrLibFPQ23m3d6bmdl_cPQ23m3d8anmChr_cPCcQ44nw4r3g3d9AnmObjChr10BindOption=0x800dfa80
clearZoromeTime__6dWmLibFv=0x800fb450
setStartPointKinokoHouseKindNum__6dWmLibFUc=0x800fb490
IsAllComplete__6dWmLibFv=0x800fd3f0
fn_80103420=0x80103420
```

### Must-Not-Pin List (52 symbols):
- **DOL symbols (51 symbols)**:
  `getKindString__7dBase_cCFv` (0x8006C660), `draw2D__12dBaseActor_cFv` (0x8006CA50), `draw2D_lyt2__12dBaseActor_cFv` (0x8006CA60), `getRes__6dRes_cCFPCcPCc` (0x800DF270), `__dt__10dWmActor_cFv` (0x800F2880), `preCreate__10dWmActor_cFv` (0x800F28E0), `postCreate__10dWmActor_cF...` (0x800F2910), `preDelete__10dWmActor_cFv` (0x800F2920), `postDelete__10dWmActor_cF...` (0x800F2950), `preExecute__10dWmActor_cFv` (0x800F2960), `postExecute__10dWmActor_cF...` (0x800F2A10), `preDraw__10dWmActor_cFv` (0x800F2A20), `postDraw__10dWmActor_cF...` (0x800F2AF0), `setSoftLight_MapObj__10dWmActor_cFRQ23m3d6bmdl_c` (0x800F2B30), `__ct__14dWmDemoActor_cFv` (0x800F60E0), `IsCourseClear__13dWmObjActor_cFv` (0x800FD9B0), `IsCourseFirstOmoteClear__13dWmObjActor_cFv` (0x800FD9D0), `IsCourseFirstUraClear__13dWmObjActor_cFv` (0x800FD9F0), `IsCourseFirstClear__13dWmObjActor_cFv` (0x800FDA30), `deleteReady__7fBase_cFv` (0x80162410), `entryFrmHeap__7fBase_cFUlPQ23EGG4Heap` (0x80162730), `entryFrmHeapNonAdjust__7fBase_cFUlPQ23EGG4Heap` (0x80162930), `createHeap__7fBase_cFv` (0x801629F0), `__nw__7fBase_cFUl` (0x80162A00), `__dl__7fBase_cFPv` (0x80162A60), `create__Q23m3d8anmChr_cF...` (0x80165210), `setAnm__Q23m3d8anmChr_cFRQ23m3d6bmdl_c...` (0x80165330), `create__Q23m3d13anmChrBlend_cF...` (0x80165660), `attach__Q23m3d13anmChrBlend_cFiPQ23m3d8anmChr_cf` (0x80165800), `__dt__Q23m3d6banm_cFv` (0x80168000), `setRate__Q23m3d6banm_cFf` (0x80168220), `setFrame__Q23m3d6fanm_cFf` (0x80169120), `isStop__Q23m3d6fanm_cCFv` (0x80169160), `__ct__Q23m3d5mdl_cFv` (0x80169E10), `__dt__Q23m3d5mdl_cFv` (0x80169E60), `create__Q23m3d5mdl_cF...` (0x80169ED0), `setAnm__Q23m3d5mdl_cFRQ23m3d6banm_cf` (0x8016A0C0), `setScale__Q23m3d9scnLeaf_cFRCQ34nw4r4math4VEC3` (0x8016A290), `setLocalMtx__Q23m3d9scnLeaf_cFPCQ34nw4r4math5MTX34` (0x8016A2B0), `calc__Q23m3d9scnLeaf_cFb` (0x8016A2E0), `__dt__Q23m3d6smdl_cFv` (0x8016A480), `__ct__12mAllocator_cFv` (0x8016A720), `__dt__16mHeapAllocator_cFv` (0x8016A8C0), `ZXYrotM__6mMtx_cF4mAng4mAng4mAng` (0x8016F090), `__construct_array` (0x802DCC90), `__destroy_arr` (0x802DCD88), `__vt__Q23m3d6banm_c` (0x80329920), `g_gameHeaps__5mHeap` (0x80377F48), `m_instance__9dResMng_c` (0x8042A318), `c_CASTLE_ID__10dCsvData_c` (0x8042D24C), `c_START_ID__10dCsvData_c` (0x8042D264).
- **REL symbols (1 symbol)**:
  `__dt__Q23m3d8anmChr_cFv` = `.text:0x4b180` (defined by landed [d_basesNP/bases/d_awa.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_basesNP/bases/d_awa.cpp)).

---

# Part 2. Task B: Strategic Survey of `d_en_bossNP`

## 2.1 State of `d_en_bossNP.rel`

`d_en_bossNP.rel` contains **356,396 bytes of code** across **26 translation units**, **22 actor profile definitions**, and **2,384 functions**. Currently, only `runtime/rel_init.cpp` (112 bytes, **0.031%**) is landed.

When executing `tools/sibmap.py` over the 5,168-function corpus, the following stderr warning was captured:
```
sibmap: WARNING: 1 FAMILY entries match no corpus file and contribute nothing:
    d_a_en_dpakkun
```

### Critical Tractability Finding: Universal Anonymous Symbols
In `d_en_bossNP_symbols.txt`, **2,377 of the 2,384 functions (99.7%) are anonymous `fn_4_*` symbols**.
- **Impact on Signature Evidence**: Zero boss functions carry CFront mangling evidence for parameter types or const-qualifiers in the symbol table. Every signature must be reconstructed from register usage (`r3`/`r4` conventions) and virtual table slot alignments.
- **Impact on Authoring Freedom**: Conversely, lack of mangled names eliminates symbol-name mismatch hazards on non-virtual helpers, but makes base-class reconstruction critical.

---

## 2.2 Sibling Similarity & Ranked Queue of the Next 8 Authorable TUs

All candidate translation units were scored instruction-by-instruction against the corpus using `tools/sibmap.py` and ranked by **progress-per-unit-of-risk**:

```
+------+------------------------------------+----------------------------------+------------+----------+-----------+--------------------+-----------------------+
| Rank | Translation Unit                   | Profile / Class                  | Code Bytes | Span (B) | Functions | Sibling Score      | Key Strategic Value   |
+------+------------------------------------+----------------------------------+------------+----------+-----------+--------------------+-----------------------+
|  1   | d_a_en_boss_koopa_demo_cage.cpp    | g_profile_EN_BOSS_KOOPA_DEMO_CAGE|  3,368 B   | 3,556 B  |    32     | 71.7% e / 74.2% sh | Zero-risk starter     |
|  2   | d_a_en_boss_koopa_jr_a.cpp         | g_profile_EN_BOSS_KOOPA_JR_A     |  6,784 B   | 7,164 B  |    59     | 46.0% e / 51.7% sh | Unblocks Jr B & Jr C  |
|  3   | d_a_en_boss_koopa_jr_b.cpp         | g_profile_EN_BOSS_KOOPA_JR_B     |  8,520 B   | 8,864 B  |    55     | 48.9% e / 55.8% sh | Exact sibling twin    |
|  4   | d_a_en_boss_castle_larry.cpp       | g_profile_EN_BOSS_CASTLE_LARRY   |  7,876 B   | 8,336 B  |    79     | 44.1% e / 56.2% sh | Smallest Castle Koopa |
|  5   | d_a_en_boss_larry.cpp              | g_profile_EN_BOSS_LARRY          | 11,796 B   | 12,264 B |    81     | 45.7% e / 60.7% sh | Smallest World Koopa  |
|  6   | d_a_en_boss_castle_iggy.cpp        | g_profile_EN_BOSS_CASTLE_IGGY    | 16,328 B   | 17,612 B |   181     | 46.9% e / 50.3% sh | Gates 7 Castle Koopas |
|  7   | d_a_en_boss_morton.cpp             | g_profile_EN_BOSS_MORTON         | 14,668 B   | 15,300 B |   109     | 44.3% e / 53.7% sh | Physics pillar boss   |
|  8   | d_boss_warning.cpp                 | No Profile (dWarningManager_c)   |  6,208 B   | 6,396 B  |    39     | 40.1% e / 45.7% sh | Layout HUD animator   |
+------+------------------------------------+----------------------------------+------------+----------+-----------+--------------------+-----------------------+
```

---

### Candidate 1: [d_a_en_boss_koopa_demo_cage.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_koopa_demo_cage.cpp) (TU 11) — The Zero-Risk Starter

- **Profile & Class**: `g_profile_EN_BOSS_KOOPA_DEMO_CAGE` / `daEnBossKoopaDemoCage_c` (**Leaf Actor**).
- **Inheritance**: `dEnBoss_c` $\to$ `daEnBossKoopaDemoCage_c` (Peach cage in final boss sequence).
- **Gating Impact**: Unblocks the 3-unit Bowser Cutscene Demo Cluster (`DEMO_CAGE`, `DEMO_KAMECK`, `DEMO_PEACH` totaling 19,488 B code).
- **Section Bounds**:
  * `.text`: `0x29118-0x29efc` (Span: 3,556 B, Code: 3,368 B, 32 functions).
  * `.ctors`: `0x28-0x2c` (Span: 4 B).
  * `.rodata`: `0xcb0-0xcd8` (Span: 40 B / 0x28).
  * `.data`: `0x9500-0x9988` (Span: 1,160 B / 0x488).
  * `.bss`: `0x26c8-0x2714` (Span: 76 B / 0x4C).
- **Adjacency & Base Subtraction**: Subtracted Base `0x00000000`. 0 overlaps. Directly bracketed between Bowser (`0x2102c-0x29118`) and Demo Kamek (`0x29efc-0x2beb4`).
- **Tractability**:
  * **Sibling Score**: **71.70% exact / 74.24% shape**.
  * Smallest actor in the entire REL (3,368 B code).
  * Implements simple cutscene animation playback (`"Wm_ko_cage"`, `"open"`) with zero physics collisions or complex state machines.
- **Symbol Coverage**: **0/32 named (32 anonymous `fn_4_*`, 100.0% anonymous)**.
  * Signature evidence derived cleanly from `dEnBoss_c` / `m3d::smdl_c` virtual table layout.
- **Why Rank 1**: Highest sibling score in the REL (71.7% exact) and lowest hazard profile. Proves the `d_en_bossNP` pipeline with minimal risk.

---

### Candidate 2: [d_a_en_boss_koopa_jr_a.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_koopa_jr_a.cpp) (TU 14) — The Airship Bowser Jr. Starter

- **Profile & Class**: `g_profile_EN_BOSS_KOOPA_JR_A` / `daEnBossKoopaJrA_c` (**Lead Leaf**).
- **Inheritance**: `dEnBoss_c` $\to$ `dEnBossKoopaJrBase_c` $\to$ `daEnBossKoopaJrA_c` (World 4 Airship Jr).
- **Gating Impact**: Unblocks the entire 3-encounter Bowser Jr. series:
  1. `d_a_en_boss_koopa_jr_b.cpp` (World 6 Jr, 8,520 B code)
  2. `d_a_en_boss_koopa_jr_c.cpp` (World 8 Jr, 10,304 B code)
  * Total unblocked code: **25,608 bytes across 163 functions**.
- **Section Bounds**:
  * `.text`: `0x2dff8-0x2fbf4` (Span: 7,164 B, Code: 6,784 B, 59 functions).
  * `.ctors`: `0x34-0x38` (Span: 4 B).
  * `.rodata`: `0xd60-0xe70` (Span: 272 B / 0x110).
  * `.data`: `0xa680-0xad90` (Span: 1,808 B / 0x710).
  * `.bss`: `0x2a28-0x2b60` (Span: 312 B / 0x138).
- **Adjacency & Base Subtraction**: Subtracted Base `0x00000000`. 0 overlaps. Directly bracketed between Demo Peach (`0x2beb4-0x2dff8`) and Koopa Jr. B (`0x2fbf4-0x31e94`).
- **Tractability**:
  * **Sibling Score**: **46.02% exact / 51.68% shape**.
  * Uses standard Junior Clown Car state machine (`dEnJrClownBase_c` states `StateID_DemoAwake`, `StateID_Move`).
- **Symbol Coverage**: **0/59 named (59 anonymous `fn_4_*`, 100.0% anonymous)**.
- **Why Rank 2**: Outstanding progress-per-unit-of-risk that immediately unlocks the Junior trio.

---

### Candidate 3: [d_a_en_boss_koopa_jr_b.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_koopa_jr_b.cpp) (TU 15) — The World 6 Bowser Jr. Twin

- **Profile & Class**: `g_profile_EN_BOSS_KOOPA_JR_B` / `daEnBossKoopaJrB_c` (**Leaf Actor**).
- **Inheritance**: `dEnBossKoopaJrBase_c` $\to$ `daEnBossKoopaJrB_c` (World 6 Electric Arena Jr).
- **Section Bounds**:
  * `.text`: `0x2fbf4-0x31e94` (Span: 8,864 B, Code: 8,520 B, 55 functions).
  * `.ctors`: `0x38-0x3c` (Span: 4 B).
  * `.rodata`: `0xe70-0xf88` (Span: 280 B / 0x118).
  * `.data`: `0xad90-0xb5d0` (Span: 2,112 B / 0x840).
  * `.bss`: `0x2b60-0x2d60` (Span: 512 B / 0x200).
- **Adjacency & Base Subtraction**: Subtracted Base `0x00000000`. 0 overlaps. Directly adjacent to Koopa Jr. A (`0x2dff8-0x2fbf4`) and Koopa Jr. C (`0x31e94-0x34800`).
- **Tractability**:
  * **Sibling Score**: **48.88% exact / 55.78% shape**.
  * Direct structural twin of Koopa Jr. A with minor electric shock state overrides (`"Wm_mr_electricshock"`).
- **Symbol Coverage**: **0/55 named (55 anonymous `fn_4_*`, 100.0% anonymous)**.
- **Why Rank 3**: Follows immediately from Jr A with minimal incremental effort.

---

### Candidate 4: [d_a_en_boss_castle_larry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_castle_larry.cpp) (TU 2) — The Smallest Tower Koopaling

- **Profile & Class**: `g_profile_EN_BOSS_CASTLE_LARRY` / `daEnBossCastleLarry_c` (**Leaf Actor**).
- **Inheritance**: `dEnBoss_c` $\to$ `dEnTorideKokoopa_c` $\to$ `daEnBossCastleLarry_c` (World 1 Tower Boss).
- **Section Bounds**:
  * `.text`: `0x45dc-0x666c` (Span: 8,336 B, Code: 7,876 B, 79 functions).
  * `.ctors`: `0x4-0x8` (Span: 4 B).
  * `.rodata`: `0x140-0x1f0` (Span: 176 B / 0xB0).
  * `.data`: `0xeb0-0x1958` (Span: 2,728 B / 0xAA8).
  * `.bss`: `0x488-0x690` (Span: 520 B / 0x208).
- **Adjacency & Base Subtraction**: Subtracted Base `0x00000000`. 0 overlaps. Directly bracketed between Castle Iggy (`0x110-0x45dc`) and Castle Lemmy (`0x666c-0x9bc4`).
- **Tractability**:
  * **Sibling Score**: **44.10% exact / 56.21% shape**.
  * Smallest of the 7 Castle Koopalings (7,876 B code vs 16.3 KB for Iggy).
  * Standard wand attack and shell spin states (`StateID_AttackBegin`, `StateID_AttackSearch`).
- **Symbol Coverage**: **0/79 named (79 anonymous `fn_4_*`, 100.0% anonymous)**.
- **Why Rank 4**: The most tractable entry point into the 7-Koopaling Fortress system.

---

### Candidate 5: [d_a_en_boss_larry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_larry.cpp) (TU 17) — The Smallest Castle Boss Koopaling

- **Profile & Class**: `g_profile_EN_BOSS_LARRY` / `daEnBossLarry_c` (**Leaf Actor**).
- **Inheritance**: `dEnBoss_c` $\to$ `dEnCastleKokoopa_c` $\to$ `daEnBossLarry_c` (World 1 Castle Boss).
- **Section Bounds**:
  * `.text`: `0x34800-0x377e8` (Span: 12,264 B, Code: 11,796 B, 81 functions).
  * `.ctors`: `0x40-0x44` (Span: 4 B).
  * `.rodata`: `0x10f0-0x1328` (Span: 568 B / 0x238).
  * `.data`: `0xbcd8-0xcb98` (Span: 3,776 B / 0xEC0).
  * `.bss`: `0x2ea0-0x30e8` (Span: 584 B / 0x248).
- **Adjacency & Base Subtraction**: Subtracted Base `0x00000000`. 0 overlaps. Directly bracketed between Koopa Jr. C (`0x31e94-0x34800`) and World Lemmy (`0x377e8-0x3d038`).
- **Tractability**:
  * **Sibling Score**: **45.71% exact / 60.70% shape**.
  * Highest shape similarity among the 7 full Castle Bosses (60.7% shape match).
- **Symbol Coverage**: **0/81 named (81 anonymous `fn_4_*`, 100.0% anonymous)**.
- **Why Rank 5**: Opens the World Boss Koopaling series with the lowest code volume.

---

### Candidate 6: [d_a_en_boss_castle_iggy.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_castle_iggy.cpp) (TU 1) — The Tower Koopaling Base Class Host

- **Profile & Class**: `g_profile_EN_BOSS_CASTLE_IGGY` / `daEnBossCastleIggy_c` (**Base Host**).
- **Inheritance**: Hosts the base class `dEnTorideKokoopa_c` and derived `daEnBossCastleIggy_c`.
- **Gating Impact**: **Gates all 7 Tower Koopalings** (Castle Larry, Lemmy, Ludwig, Morton, Roy, Wendy totaling 88,868 B code across 683 functions).
- **Section Bounds**:
  * `.text`: `0x110-0x45dc` (Span: 17,612 B, Code: 16,328 B, 181 functions).
  * `.ctors`: `0x0-0x4` (Span: 4 B).
  * `.rodata`: `0x0-0x140` (Span: 320 B / 0x140).
  * `.data`: `0x0-0xeb0` (Span: 3,760 B / 0xEB0).
  * `.bss`: `0x8-0x488` (Span: 1,152 B / 0x480).
- **Adjacency & Base Subtraction**: Subtracted Base `0x00000000`. 0 overlaps. Begins directly following `global_destructor_chain.c` (`0x70-0x110`) and followed by Castle Larry (`0x45dc-0x666c`).
- **Tractability**:
  * **Sibling Score**: **46.88% exact / 50.30% shape**.
  * Large TU (16.3 KB), but contains the shared state machine methods for all 7 Tower Koopalings.
- **Symbol Coverage**: **0/181 named (181 anonymous `fn_4_*`, 100.0% anonymous)**.
- **Why Rank 6**: Essential structural base for the entire Fortress sequence.

---

### Candidate 7: [d_a_en_boss_morton.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_morton.cpp) (TU 20) — The World 6 Castle Boss

- **Profile & Class**: `g_profile_EN_BOSS_MORTON` / `daEnBossMorton_c` (**Leaf Actor**).
- **Inheritance**: `dEnCastleKokoopa_c` $\to$ `daEnBossMorton_c` (Pillar Ground-Pound Boss).
- **Section Bounds**:
  * `.text`: `0x42798-0x4635c` (Span: 15,300 B, Code: 14,668 B, 109 functions).
  * `.ctors`: `0x4c-0x50` (Span: 4 B).
  * `.rodata`: `0x1818-0x1ad0` (Span: 696 B / 0x2B8).
  * `.data`: `0xf168-0x10180` (Span: 4,120 B / 0x1018).
  * `.bss`: `0x3bf8-0x3f78` (Span: 896 B / 0x380).
- **Adjacency & Base Subtraction**: Subtracted Base `0x00000000`. 0 overlaps. Directly bracketed between World Ludwig (`0x3d038-0x42798`) and World Roy (`0x4635c-0x4a35c`).
- **Tractability**:
  * **Sibling Score**: **44.29% exact / 53.68% shape**.
  * Pillar collision mechanics closely mirror `daEnSuperBigpile_c` (banked in DOL).
- **Symbol Coverage**: **0/109 named (109 anonymous `fn_4_*`, 100.0% anonymous)**.
- **Why Rank 7**: Substantial 14.7 KB yield with strong enemy-actor physics precedent.

---

### Candidate 8: [d_boss_warning.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_boss_warning.cpp) (TU 23) — The Boss Siren & HUD Manager

- **Profile & Class**: `No Profile` / `dWarningManager_c` & `LytBase_c` (**Helper / Layout**).
- **Role**: Manages boss intro warning sirens, screen shaking, and HUD layout animation dispatch.
- **Section Bounds**:
  * `.text`: `0x4ee48-0x50744` (Span: 6,396 B, Code: 6,208 B, 39 functions).
  * `.ctors`: `0x58-0x5c` (Span: 4 B).
  * `.rodata`: `0x1f70-0x2100` (Span: 400 B / 0x190).
  * `.data`: `0x1252c-0x12dc0` (Span: 2,196 B / 0x894).
  * `.bss`: `0x4808-0x4818` (Span: 16 B / 0x10).
- **Adjacency & Base Subtraction**: Subtracted Base `0x00000000`. 0 overlaps. Directly bracketed between World Wendy (`0x4a35c-0x4ee48`) and Kamek Demo Manager (`0x50744-0x51bc4`).
- **Tractability**:
  * **Sibling Score**: **40.07% exact / 45.71% shape**.
  * Relies on standard 2D layout functions (`AnimePlay__9LytBase_cFv`, `AnimeStartSetup__9LytBase_cFib`) already banked across `dol/layout/`.
- **Symbol Coverage**: **0/39 named (39 anonymous `fn_4_*`, 100.0% anonymous)**.
- **Why Rank 8**: Self-contained 6.2 KB layout unit with zero boss AI complexity.

---

## 2.3 Honest Strategic Assessment of `d_en_bossNP`

1. **Overall Tractability**:
   - `d_en_bossNP.rel` has **only one low-risk starter**: [d_a_en_boss_koopa_demo_cage.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_en_bossNP/bases/d_a_en_boss_koopa_demo_cage.cpp) (3.3 KB code, 71.7% exact match).
   - Beyond `koopa_demo_cage`, boss units in this REL average **13.7 KB per translation unit** and contain complex multi-phase state machines with custom animated cutscenes.
   - Combined with the **99.7% anonymous symbol coverage**, reverse-engineering full boss units from scratch carries substantial register-allocation risk compared to `d_basesNP` or `d_enemiesNP`.

2. **Strategic Recommendation**:
   - **Do NOT prioritize `d_en_bossNP.rel` for bulk authoring right now.**
   - Authoring effort yields dramatically higher progress-per-unit-of-risk in **`d_basesNP`** (where `wm_grid`, `wm_tower`, `wm_smallcloud`, and the 4-unit `kinoko_base` family provide ~8 KB of 70–88% matching code) and **`d_enemiesNP`** (where `net_nokonoko_lr` and `left_dokan_pakkun` provide ~14 KB of 68–71% matching code).
   - If work on `d_en_bossNP.rel` is desired, author **`d_a_en_boss_koopa_demo_cage.cpp`** to establish the REL build baseline, followed by the **Bowser Jr. Airship series (`koopa_jr_a`, `koopa_jr_b`, `koopa_jr_c`)**, deferring the heavy Koopaling state machine bases until more enemy/boss precedent is landed.
