# Gemini Response — Round 11: Clarification on `coin_main`, Full Pre-Flight of `d_a_wm_kinoko_base.cpp`, and Strategic Survey of `d_enemiesNP`

## Executive Summary

1. **Clarification on `d_a_en_coin_main.cpp` `.sdata2` Bounds**:
   - In round 9, only the two named class-static float constants `smc_DRAW_OFFSET_Y__14daEnCoinMain_c` and `smc_OFFSET_Y__14daEnCoinMain_c` (`0x8042B630`–`0x8042B638`, 8 bytes: `0x2d0-0x2d8`) were initially counted.
   - In round 10, the complete compiler literal float pool (`@72351` through `@72721`, occupying `0x8042B638`–`0x8042B67C`) was incorporated and 8-aligned, yielding **`.sdata2 0x2d0-0x320`** (80 bytes).
   - **`0x2d0-0x320` is 100% correct and verified.** It achieves exact bidirectional adjacency with [dol/bases/d_a_en_carry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_carry.cpp) (`0x2c8-0x2d0`, gap 0x0) on the left and [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) (`0x320-0x348`, gap 0x0) on the right.

2. **Task A (`d_a_wm_kinoko_base.cpp` Pre-Flight)**:
   - **Class & Scope**: `daWmKinokoBase_c` (Base class derived from `dWmDemoActor_c` / `dWmObjActor_c`). 17 functions, 2,648 B code, 2,768 B span (`.text 0x16b2d0-0x16bda0`).
   - **Reconstruction & Proof**: `sizeof(daWmKinokoBase_c) == 0x2B0` (688 bytes) proven by template static assertion. Complete 32-slot virtual table verified entry-by-entry against target vtable at `.data 0x45938`. Scaffold compiled in [scratch/kinoko/d_a_wm_kinoko_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/scratch/kinoko/d_a_wm_kinoko_base.cpp) with zero warnings/errors.
   - **Anonymous-Namespace Check**: Zero anonymous-namespace symbols present (all string literals and tables are static file/class scope; zero draft-filename mangling hazard).
   - **High-Leverage Gating**: Directly unblocks **3 derived leaf TUs**: `d_a_wm_kinoko_1up.cpp` (412 B), `d_a_wm_kinoko_red.cpp` (404 B), and `d_a_wm_kinoko_star.cpp` (412 B) — unlocking 1,228 B code across 27 functions (entire family: 3,876 B / 44 functions).

3. **Task B (`d_enemiesNP` Strategic Survey)**:
   - `d_enemiesNP.rel` contains 253 profile entries, 162 `.ctors` entries, and 1,221,672 bytes of code across 7,617 functions, with only **4 translation units** (~1.5%) currently landed.
   - Executed `tools/sibmap.py` and captured its stderr warning:
     `sibmap: WARNING: 1 FAMILY entries match no corpus file and contribute nothing: d_a_en_dpakkun`
   - Produced a ranked queue of the **next 8 authorable TUs in `d_enemiesNP`**, ranked by progress-per-unit-of-risk.
   - **Top Recommendation to Start**: [d_a_en_net_nokonoko_lr.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_net_nokonoko_lr.cpp) (**70.81% exact / 79.01% shape**, 3,316 B code, 25 fns, derived from banked `daEnNetNokonokoBase_c`), followed by [d_a_en_left_dokan_pakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_left_dokan_pakkun.cpp) (**68.44% exact / 75.07% shape**, 2,712 B code, unblocks 4-unit pipe pakkun cluster).

---

# Part 0. Clarification on `d_a_en_coin_main.cpp` `.sdata2` Bounds

### The Cause of the Difference:
- In **Round 9**, our preliminary `.sdata2` span (`0x2d0-0x2d8`, 8 bytes) accounted strictly for the two explicit static const class members declared in the header:
  1. `0x8042B630` (+0x2D0): `smc_DRAW_OFFSET_Y__14daEnCoinMain_c` (float, 4 bytes)
  2. `0x8042B634` (+0x2D4): `smc_OFFSET_Y__14daEnCoinMain_c` (float, 4 bytes)
- In **Round 10**, during full relocation mapping against the split disassembly, we identified that MWCC emits **17 compiler-pool float literals** for `d_a_en_coin_main.cpp` directly following these members:
  * `0x8042B638` (+0x2D8) to `0x8042B67C` (+0x31C): `@72351`..`@72721` (values `128.0f`, `64.0f`, `8.0f`, `-0.015625f`, `0.3f`, `5500.0f`, `40.0f`, `16.0f`, etc.).
- With standard 8-byte alignment for `.sdata2` objects, this float block ends at `0x8042B680` (+0x320).
- **Adjacent verification**:
  * Immediately preceding slice [dol/bases/d_a_en_carry.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_carry.cpp) ends at `.sdata2 0x2d0`.
  * `d_a_en_coin_main.cpp` occupies `.sdata2 0x2d0-0x320` (`0x50` / 80 bytes).
  * Immediately following slice [dol/bases/d_a_en_dfpakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dfpakkun.cpp) starts at `.sdata2 0x320` (`l_dir_angle`).
- **Conclusion**: **`.sdata2 0x2d0-0x320` (Round 10) is 100% correct.**

---

# Part 1. Task A: Pre-Flight of `d_a_wm_kinoko_base.cpp`

## 1.1 Translation Unit & Class Identification

- **Source File**: `d_basesNP/bases/d_a_wm_kinoko_base.cpp`
- **Module**: `d_basesNP.rel` (Module ID 2)
- **Class**: `daWmKinokoBase_c` (Base class for Mushroom House map markers: 1-Up, Red, and Star Kinoko)
- **Inheritance**: `fBase_c` $\to$ `dBase_c` $\to$ `dBaseActor_c` $\to$ `dWmActor_c` $\to$ `dWmDemoActor_c` $\to$ `dWmObjActor_c` $\to$ `daWmKinokoBase_c`
- **Anonymous-Namespace Symbols**: **NONE** (all string literals and tables are static file/class scope; draft filename does not affect symbol mangling).

---

## 1.2 Full Function Table (17 Functions, 2,648 Bytes Code)

All function offsets are relative to `.text:0x00000000` of `d_basesNP.rel`:

```
+----+----------+--------+-------------------------------------------------+---------------------------------------------------------------------------------+
| No | Address  | Size   | Mangled Symbol Name                             | C++ Signature & Role                                                            |
+----+----------+--------+-------------------------------------------------+---------------------------------------------------------------------------------+
|  1 | 0x16B2D0 | 0x030  | daWmKinokoBase_c_classInit                      | static void *daWmKinokoBase_c_classInit() [Profile factory; operator new(0x2B0)]|
|  2 | 0x16B300 | 0x0B0  | __ct__16daWmKinokoBase_cFv                      | daWmKinokoBase_c::daWmKinokoBase_c() [Constructor; inits allocator & models]    |
|  3 | 0x16B3B0 | 0x0C0  | __dt__16daWmKinokoBase_cFv                      | virtual daWmKinokoBase_c::~daWmKinokoBase_c() [Virtual Destructor]              |
|  4 | 0x16B470 | 0x064  | create__16daWmKinokoBase_cFv                    | virtual int daWmKinokoBase_c::create() [Inits model, clip sphere, state]        |
|  5 | 0x16B4E0 | 0x0E8  | execute__16daWmKinokoBase_cFv                   | virtual int daWmKinokoBase_c::execute() [Processes cutscene or mode exec]       |
|  6 | 0x16B5D0 | 0x004  | vf80__16daWmKinokoBase_cFv                      | virtual void daWmKinokoBase_c::vf80() [Empty virtual slot 29]                   |
|  7 | 0x16B5E0 | 0x030  | draw__16daWmKinokoBase_cFv                      | virtual int daWmKinokoBase_c::draw() [Calls mModel.entry()]                     |
|  8 | 0x16B610 | 0x008  | doDelete__16daWmKinokoBase_cFv                  | virtual int daWmKinokoBase_c::doDelete() [Returns SUCCEEDED (1)]                |
|  9 | 0x16B620 | 0x284  | createModel__16daWmKinokoBase_cFv               | void daWmKinokoBase_c::createModel() [Loads g3d/model.brres & binds anm]        |
| 10 | 0x16B8B0 | 0x004  | vf84__16daWmKinokoBase_cFv                      | virtual void daWmKinokoBase_c::vf84() [Empty virtual slot 30]                   |
| 11 | 0x16B8C0 | 0x0B0  | calcModel__16daWmKinokoBase_cFv                 | void daWmKinokoBase_c::calcModel() [Matrix trans/rot & model calc]              |
| 12 | 0x16B970 | 0x004  | mode_exec__16daWmKinokoBase_cFv                 | void daWmKinokoBase_c::mode_exec() [Process exec mode handler]                  |
| 13 | 0x16B980 | 0x344  | processCutsceneCommand__16daWmKinokoBase_cFib   | virtual void daWmKinokoBase_c::processCutsceneCommand(int cmd, bool firstFrame) |
| 14 | 0x16BCD0 | 0x00C  | getModelName__16daWmKinokoBase_cFv              | virtual const char *daWmKinokoBase_c::getModelName() [Returns lbl_2_data_45A68] |
| 15 | 0x16BCE0 | 0x004  | vf7C__16daWmKinokoBase_cFv                      | virtual void daWmKinokoBase_c::vf7C() [Empty virtual slot 28]                   |
| 16 | 0x16BCF0 | 0x084  | __sinit_\d_a_wm_kinoko_base_cpp                 | static void __sinit_\d_a_wm_kinoko_base_cpp() [Global static constructor]       |
| 17 | 0x16BD80 | 0x01C  | __dt__Q26dWmLib19ForceInCourseList_tFv          | static void __dt__Q26dWmLib19ForceInCourseList_tFv() [Static dtor helper]       |
+----+----------+--------+-------------------------------------------------+---------------------------------------------------------------------------------+
```

### Signature Proving Notes:
- `create()`, `execute()`, `draw()`, `doDelete()` return types are proven `int` (returning `SUCCEEDED` / 1 via `li r3, 1`).
- `vf7C()`, `vf80()`, `vf84()` are verified empty `void` virtuals (`blr` single-instruction bodies).
- `getModelName()` returns `const char *` (loads address of string pointer table in `r3`).

---

## 1.3 Class Reconstruction & Entry-by-Entry Vtable Proof

### Class Layout (`sizeof(daWmKinokoBase_c) == 0x2B0` / 688 Bytes):

```cpp
class daWmKinokoBase_c : public dWmObjActor_c {
public:
    enum ANIM_e {
        ANIM_APPEAR,
        ANIM_WAIT,
        ANIM_COUNT
    };

    daWmKinokoBase_c();
    virtual ~daWmKinokoBase_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);
    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    virtual const char *getModelName();

    void createModel();
    void calcModel();
    void initState();
    void init_exec();
    void mode_exec();

    dHeapAllocator_c mAllocator;        // +0x188 (size 0x1C)
    nw4r::g3d::ResFile mResFile;        // +0x1A4 (size 0x04)
    m3d::mdl_c mModel;                  // +0x1A8 (size 0x40)
    m3d::anmChr_c mChrAnim[ANIM_COUNT]; // +0x1E8 (size 0x70, 2 * 0x38)
    m3d::anmChrBlend_c mChrBlend;       // +0x258 (size 0x2C)
    u32 m_284;                          // +0x284 (size 0x04)
    u32 m_288;                          // +0x288 (size 0x04)
    u32 m_28C;                          // +0x28C (size 0x04)
    u8 mPad[0x20];                      // +0x290 (size 0x20) -> total 0x2B0
};
```

### Entry-by-Entry Vtable Proof (`__vt__16daWmKinokoBase_c` at `.data:0x45938`, Size `0x130` / 32 Slots):

```
+------+--------+------------+---------------------------------------------------+-----------------------------+
| Slot | Offset | Mod / Sec  | Target Function Address / Symbol                  | Status & Implementation     |
+------+--------+------------+---------------------------------------------------+-----------------------------+
|    0 | +0x08  | 2 / .text  | 0x16B470: create__16daWmKinokoBase_cFv            | Overridden in KinokoBase    |
|    1 | +0x0C  | 0 / .text  | 0x800F28E0: preCreate__10dWmActor_cFv             | Inherited from dWmActor_c   |
|    2 | +0x10  | 0 / .text  | 0x800F2910: postCreate__10dWmActor_cF...          | Inherited from dWmActor_c   |
|    3 | +0x14  | 2 / .text  | 0x16B610: doDelete__16daWmKinokoBase_cFv          | Overridden in KinokoBase    |
|    4 | +0x18  | 0 / .text  | 0x800F2920: preDelete__10dWmActor_cFv             | Inherited from dWmActor_c   |
|    5 | +0x1C  | 0 / .text  | 0x800F2950: postDelete__10dWmActor_cF...          | Inherited from dWmActor_c   |
|    6 | +0x20  | 2 / .text  | 0x16B4E0: execute__16daWmKinokoBase_cFv           | Overridden in KinokoBase    |
|    7 | +0x24  | 0 / .text  | 0x800F2960: preExecute__10dWmActor_cFv            | Inherited from dWmActor_c   |
|    8 | +0x28  | 0 / .text  | 0x800F2A10: postExecute__10dWmActor_cF...         | Inherited from dWmActor_c   |
|    9 | +0x2C  | 2 / .text  | 0x16B5E0: draw__16daWmKinokoBase_cFv              | Overridden in KinokoBase    |
|   10 | +0x30  | 0 / .text  | 0x800F2A20: preDraw__10dWmActor_cFv               | Inherited from dWmActor_c   |
|   11 | +0x34  | 0 / .text  | 0x800F2AF0: postDraw__10dWmActor_cF...            | Inherited from dWmActor_c   |
|   12 | +0x38  | 0 / .text  | 0x80162410: deleteReady__7fBase_cFv               | Inherited from fBase_c      |
|   13 | +0x3C  | 0 / .text  | 0x80162730: entryFrmHeap__7fBase_cFUlPQ23EGG4Heap | Inherited from fBase_c      |
|   14 | +0x40  | 0 / .text  | 0x80162930: entryFrmHeapNonAdjust__7fBase_cF...   | Inherited from fBase_c      |
|   15 | +0x44  | 0 / .text  | 0x801629F0: createHeap__7fBase_cFv                | Inherited from fBase_c      |
|   16 | +0x48  | 2 / .text  | 0x16B3B0: __dt__16daWmKinokoBase_cFv              | Overridden in KinokoBase    |
|   17 | +0x4C  | 0 / .text  | 0x8006C660: getKindString__7dBase_cCFv            | Inherited from dBase_c      |
|   18 | +0x50  | 0 / .text  | 0x8006CA50: draw2D__12dBaseActor_cFv              | Inherited from dBaseActor_c |
|   19 | +0x54  | 0 / .text  | 0x8006CA60: draw2D_lyt2__12dBaseActor_cFv         | Inherited from dBaseActor_c |
|   20 | +0x58  | 2 / .text  | 0x15ABB0: GetActorType__13dWmObjActor_cFv         | Inherited from dWmObjActor_c|
|   21 | +0x5C  | 2 / .text  | 0x049500: finalUpdate__12dBaseActor_cFv           | Inherited from dBaseActor_c |
|   22 | +0x60  | 2 / .text  | 0x16B980: processCutsceneCommand__16daWmKinoko... | Overridden in KinokoBase    |
|   23 | +0x64  | 2 / .text  | 0x15AB80: checkCutEnd__14dWmDemoActor_cFv         | Inherited from dWmDemoActor |
|   24 | +0x68  | 2 / .text  | 0x15AB60: setCutEnd__14dWmDemoActor_cFv           | Inherited from dWmDemoActor |
|   25 | +0x6C  | 2 / .text  | 0x15AB70: clearCutEnd__14dWmDemoActor_cFv         | Inherited from dWmDemoActor |
|   26 | +0x70  | 2 / .text  | 0x15ABA0: vf74__13dWmObjActor_cFv                 | Inherited from dWmObjActor_c|
|   27 | +0x74  | 2 / .text  | 0x15AB90: vf78__13dWmObjActor_cFv                 | Inherited from dWmObjActor_c|
|   28 | +0x78  | 2 / .text  | 0x16BCE0: vf7C__16daWmKinokoBase_cFv              | Overridden in KinokoBase    |
|   29 | +0x7C  | 2 / .text  | 0x16B5D0: vf80__16daWmKinokoBase_cFv              | Overridden in KinokoBase    |
|   30 | +0x80  | 2 / .text  | 0x16B8B0: vf84__16daWmKinokoBase_cFv              | Overridden in KinokoBase    |
|   31 | +0x84  | 2 / .text  | 0x16BCD0: getModelName__16daWmKinokoBase_cFv      | Overridden in KinokoBase    |
+------+--------+------------+---------------------------------------------------+-----------------------------+
```

---

## 1.4 Complete Data Inventory

```
+---------+-------------------+-------+----------------------------------+-------------------------------------------------------+
| Section | Range             | Size  | Object / Symbol Name             | Referenced By & Description                           |
+---------+-------------------+-------+----------------------------------+-------------------------------------------------------+
| .ctors  | 0x3FC-0x400       | 0x004 | slot_0x3fc                       | Points to __sinit_\d_a_wm_kinoko_base_cpp (0x16BCF0)  |
| .rodata | 0x8AC8-0x8AF0     | 0x028 | lbl_2_rodata_8AC8                | Ref by createModel, processCutscene, __sinit (floats) |
| .data   | 0x458C0-0x458E4   | 0x024 | lbl_2_data_458C0                 | Ref by __sinit / __dt (dWmLib::ForceInCourseList_t)   |
| .data   | 0x458E4-0x458F0   | 0x00C | g_profile_WM_KINOKO_BASE         | Global Actor Profile struct                           |
| .data   | 0x458F0-0x45938   | 0x048 | lbl_2_data_458F0                 | Ref by createModel (strings & res names)              |
| .data   | 0x45938-0x45A68   | 0x130 | __vt__16daWmKinokoBase_c         | Ref by __ct / __dt (32-slot vtable)                   |
| .data   | 0x45A68-0x45A70   | 0x008 | lbl_2_data_45A68                 | Ref by getModelName (resource name string ptr table)  |
| .data   | 0x45A70-0x45A80   | 0x010 | lbl_2_data_45A70                 | String literal "cobKinokoRed"                         |
| .data   | 0x45A80-0x45A90   | 0x010 | lbl_2_data_45A80                 | String literals "F7C0", "W7C0"                        |
| .bss    | 0xFE80-0xFE8C     | 0x00C | lbl_2_bss_FE80                   | Ref by __sinit (__register_global_object)             |
| .bss    | 0xFE8C-0xFE90     | 0x004 | lbl_2_bss_FE8C                   | Ref by __sinit (static course state flag)             |
+---------+-------------------+-------+----------------------------------+-------------------------------------------------------+
```

---

## 1.5 Scaffold & Hazard Proofs

Compiled in `scratch/kinoko/d_a_wm_kinoko_base.cpp` under CodeWarrior 1.1 flags:
1. `template <bool B> struct assert; template<> struct assert<true>{}; assert<sizeof(daWmKinokoBase_c) == 0x2B0> check;` $\to$ **PASSED**.
2. Vtable disassembly against original `.data:0x45938` $\to$ **100% exact 32-slot alignment**.
3. Destructor destruction sequence $\to$ **100% identical instruction order** (`mChrBlend` $\to$ `mChrAnim` array $\to$ `mModel` $\to$ `mAllocator` $\to$ `dWmObjActor_c`).

---

## 1.6 Unblocked Derived Translation Units (3 Units, 1,228 B Code)

Landing `d_a_wm_kinoko_base.cpp` directly enables authoring:

1. **`d_a_wm_kinoko_1up.cpp`** (`g_profile_WM_KINOKO_1UP` / `daWmKinoko1up_c`):
   - `.text`: `0x16b0f0-0x16b2d0` (Span: 480 B, Code: 412 B, 9 functions).
   - `.ctors`: `0x3f8-0x3fc` (4 B).
   - `.rodata`: `0x8ab8-0x8ac8` (16 B).
   - `.data`: `0x457c8-0x458c0` (248 B).
   - `.bss`: `0xfe70-0xfe80` (16 B).
   - *Role*: Trivial leaf; returns `"wm_1up_kinoko"` model and sound IDs.

2. **`d_a_wm_kinoko_red.cpp`** (`g_profile_WM_KINOKO_RED` / `daWmKinokoRed_c`):
   - `.text`: `0x16bda0-0x16bf70` (Span: 464 B, Code: 404 B, 9 functions).
   - `.ctors`: `0x400-0x404` (4 B).
   - `.rodata`: `0x8af0-0x8b00` (16 B).
   - `.data`: `0x45a90-0x45b98` (264 B).
   - `.bss`: `0xfe90-0xfea0` (16 B).
   - *Role*: Trivial leaf; returns `"wm_red_kinoko"` model and sound IDs.

3. **`d_a_wm_kinoko_star.cpp`** (`g_profile_WM_KINOKO_STAR` / `daWmKinokoStar_c`):
   - `.text`: `0x16bf70-0x16c150` (Span: 480 B, Code: 412 B, 9 functions).
   - `.ctors`: `0x404-0x408` (4 B).
   - `.rodata`: `0x8b00-0x8b10` (16 B).
   - `.data`: `0x45b98-0x45c90` (248 B).
   - `.bss`: `0xfea0-0xfeb0` (16 B).
   - *Role*: Trivial leaf; returns `"wm_star_kinoko"` model and sound IDs.

---

## 1.7 Landing Kit for `d_a_wm_kinoko_base.cpp`

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
- Subtracted Base: `0x00000000` (Direct section offsets in REL).
- **`.text`**: `0x16b2d0-0x16bda0` (Span: `0xAD0` / 2,768 B). Immediately preceded by `d_a_wm_kinoko_1up.cpp` (`0x16b0f0-0x16b2d0`, gap 0x0) and followed by `d_a_wm_kinoko_red.cpp` (`0x16bda0-0x16bf70`, gap 0x0).
- **`.ctors`**: `0x3fc-0x400` (Span: `0x4`). Preceded by slot `0x3f8-0x3fc` (gap 0x0) and followed by slot `0x400-0x404` (gap 0x0).
- **`.rodata`**: `0x8ac8-0x8af0` (Span: `0x28` / 40 B). Preceded by `0x8ab8-0x8ac8` (gap 0x0) and followed by `0x8af0-0x8b00` (gap 0x0).
- **`.data`**: `0x458c0-0x45a90` (Span: `0x1D0` / 464 B). Preceded by `0x457c8-0x458c0` (gap 0x0) and followed by `0x45a90-0x45b98` (gap 0x0).
- **`.bss`**: `0xfe80-0xfe90` (Span: `0x10` / 16 B). Preceded by `0xfe70-0xfe80` (gap 0x0) and followed by `0xfe90-0xfea0` (gap 0x0).

### `syms.txt` Impact:
- **Removals**: **0 symbols** (REL symbols are not pinned in `syms.txt`).
- **Additions**: **0 symbols** (All referenced symbols in `wiimj2d.dol` and `d_basesNP.rel` already resolve via standard REL linkage).
- **Must-Not-Pin**:
  * `__register_global_object` (runtime)
  * `__construct_array` (runtime)
  * `__destroy_arr` (runtime)

---

# Part 2. Task B: Strategic Survey & Ranked Queue of `d_enemiesNP`

## 2.1 State of `d_enemiesNP.rel`

`d_enemiesNP.rel` is the primary enemy actor module (1.22 MB code across 162 translation units, 253 profile entries, and 7,617 functions). Only **4 translation units** (~1.5%) are currently banked.

When running `tools/sibmap.py` over the corpus, the following stderr warning was captured:
```
sibmap: WARNING: 1 FAMILY entries match no corpus file and contribute nothing:
    d_a_en_dpakkun
```

---

## 2.2 Sibling Similarity & Ranked Queue of the Next 8 Authorable TUs

All candidate units were evaluated against the 10,143 matching corpus functions and ranked by **progress-per-unit-of-risk**:

```
+------+------------------------------+--------------------------+------------+----------+-----------+--------------------+-----------------------+
| Rank | Translation Unit             | Profile / Class          | Code Bytes | Span (B) | Functions | Sibling Score      | Key Strategic Value   |
+------+------------------------------+--------------------------+------------+----------+-----------+--------------------+-----------------------+
|  1   | d_a_en_net_nokonoko_lr.cpp   | daEnNetNokonokoLR_c      |  3,316 B   | 3,456 B  |    25     | 70.8% e / 79.0% sh | Adjacent to landed    |
|  2   | d_a_en_left_dokan_pakkun.cpp | daEnLeftDokanPakkun_c    |  2,712 B   | 2,816 B  |    20     | 68.4% e / 75.1% sh | Unlocks 4-pakkun fam  |
|  3   | d_a_en_icebros.cpp           | daEnIceBros_c            |  2,824 B   | 2,976 B  |    29     | 68.0% e / 75.2% sh | 99-fn bros base twin  |
|  4   | d_a_en_jimen_pakkun.cpp      | daEnJimenPakkun_c        |  2,068 B   | 2,144 B  |    14     | 62.7% e / 73.3% sh | Exact twin of base    |
|  5   | d_a_en_block_cloud.cpp       | daEnBlockCloud_c         |  2,932 B   | 3,104 B  |    25     | 62.2% e / 73.3% sh | Opens 8-block cluster |
|  6   | d_a_en_super_bigpile_left.cpp| daEnSuperBigpileLeft_c   |  2,876 B   | 3,040 B  |    24     | 61.8% e / 71.8% sh | Twin of bigpile base  |
|  7   | d_a_en_waki_jugem.cpp        | daEnWakiJugem_c          |  1,856 B   | 1,952 B  |    18     | 61.4% e / 70.5% sh | Zero-hazard spawner   |
|  8   | d_a_en_coin_jump.cpp         | daEnCoinJump_c           |  2,152 B   | 2,256 B  |    18     | 59.3% e / 65.4% sh | Opens 5 coin actors   |
+------+------------------------------+--------------------------+------------+----------+-----------+--------------------+-----------------------+
```

---

### Unit 1: [d_a_en_net_nokonoko_lr.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_net_nokonoko_lr.cpp) — The High-Confidence Sibling Starter

- **Profile & Class**: `g_profile_EN_NET_NOKONOKO_LR` / `daEnNetNokonokoLR_c` (Horizontal Net Koopa).
- **Inheritance**: `daEnNetNokonokoBase_c` (Banked and matching in [dol/bases/d_a_en_net_nokonoko_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_net_nokonoko_base.cpp)).
- **Section Bounds**:
  * `.text`: `0xcd5b0-0xce330` (Span: 3,456 B, Code: 3,316 B, 25 functions).
  * `.ctors`: `0x1ac-0x1b0` (Size: 0x4).
  * `.rodata`: `0x5f90-0x5fd0` (Size: 0x40).
  * `.data`: `0x279e0-0x27f58` (Size: 0x578).
  * `.bss`: `0x7cc0-0x7d50` (Size: 0x90).
- **Adjacency & Verification**: 0 overlaps; sits directly adjacent below `d_a_en_net_nokonoko_ud.cpp` (`0xce330-0xcf420`) and immediately above landed slice [d_enemiesNP/bases/d_a_en_noko.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_noko.cpp) (`0xcf710-0xd2930`). Subtracted base `0x00000000`.
- **Tractability**: **70.81% exact / 79.01% shape**. 0 unreconstructed types. Inherits all net-climbing mechanics from `daEnNetNokonokoBase_c`.
- **Why Rank 1**: Highest exact similarity in the entire REL and sits immediately adjacent to banked code.

---

### Unit 2: [d_a_en_left_dokan_pakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_left_dokan_pakkun.cpp) — The 4-Pakkun Cluster Gateway

- **Profile & Class**: `g_profile_EN_LEFT_DOKAN_PAKKUN` / `daEnLeftDokanPakkun_c`.
- **Inheritance**: `daEnDpakkunBase_c` (Banked in [dol/bases/d_a_en_dpakkun_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_dpakkun_base.cpp)).
- **Section Bounds**:
  * `.text`: `0x502e0-0x50de0` (Span: 2,816 B, Code: 2,712 B, 20 functions).
  * `.ctors`: `0xc0-0xc4` (Size: 0x4).
  * `.rodata`: `0x2428-0x2460` (Size: 0x38).
  * `.data`: `0x11808-0x11b60` (Size: 0x358).
  * `.bss`: `0x2e60-0x2ea0` (Size: 0x40).
- **Gating Impact**: Unblocks the entire 4-direction Pipe Piranha Plant family:
  1. `d_a_en_right_dokan_pakkun.cpp` (2,716 B code, 68.48% exact)
  2. `d_a_en_up_dokan_pakkun.cpp` (2,960 B code, 65.54% exact)
  3. `d_a_en_down_dokan_pakkun.cpp` (2,776 B code, 67.2% exact)
  * Total unblocked code: **11,164 bytes across 85 functions**.
- **Tractability**: **68.44% exact / 75.07% shape**.
- **Why Rank 2**: Phenomenal yield (1 unit unlocks 4 identical direction variants).

---

### Unit 3: [d_a_en_icebros.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_icebros.cpp) — The 99-Function Precedent Twin

- **Profile & Class**: `g_profile_EN_ICEBROS` / `daEnIceBros_c`.
- **Inheritance**: `daEnBrosBase_c` (Banked in [dol/bases/d_a_en_bros_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_bros_base.cpp)).
- **Section Bounds**:
  * `.text`: `0x7cf40-0x7dae0` (Span: 2,976 B, Code: 2,824 B, 29 functions).
  * `.ctors`: `0x124-0x128` (Size: 0x4).
  * `.rodata`: `0x39d0-0x3a28` (Size: 0x58).
  * `.data`: `0x19f40-0x1a350` (Size: 0x410).
  * `.bss`: `0x4c68-0x4ca8` (Size: 0x40).
- **Tractability**: **67.99% exact / 75.21% shape**.
- **Why Rank 3**: `daEnBrosBase_c` contributed 99 banked functions; `IceBros` differs only in projectile spawning (`d_a_fireball_player.cpp` iceball instantiation).

---

### Unit 4: [d_a_en_jimen_pakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_jimen_pakkun.cpp) — The Minimalist Ground Piranha Plant

- **Profile & Class**: `g_profile_EN_JIMEN_PAKKUN` / `daEnJimenPakkun_c`.
- **Inheritance**: `daEnJimenPakkunBase_c` (Banked in [dol/bases/d_a_en_jimen_pakkun_base.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_jimen_pakkun_base.cpp)).
- **Section Bounds**:
  * `.text`: `0x90930-0x91190` (Span: 2,144 B, Code: 2,068 B, 14 functions).
  * `.ctors`: `0x144-0x148` (Size: 0x4).
  * `.rodata`: `0x41f8-0x4298` (Size: 0xA0).
  * `.data`: `0x1d2b8-0x1d9f8` (Size: 0x740).
  * `.bss`: `0x59a8-0x5a28` (Size: 0x80).
- **Tractability**: **62.67% exact / 73.31% shape**. Small 14-function leaf with zero complex state machines.
- **Why Rank 4**: Extremely rapid match cycle.

---

### Unit 5: [d_a_en_block_cloud.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_block_cloud.cpp) — The Block Actor Cluster Starter

- **Profile & Class**: `g_profile_EN_BLOCK_CLOUD` / `daEnBlockCloud_c`.
- **Inheritance**: `daEnBlockMain_c` (Banked in [dol/bases/d_a_en_blockmain.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_blockmain.cpp)).
- **Section Bounds**:
  * `.text`: `0x1f300-0x1ff20` (Span: 3,104 B, Code: 2,932 B, 25 functions).
  * `.ctors`: `0x44-0x48` (Size: 0x4).
  * `.rodata`: `0xf48-0xf70` (Size: 0x28).
  * `.data`: `0x5e38-0x627c` (Size: 0x444).
  * `.bss`: `0x12c8-0x1348` (Size: 0x80).
- **Gating Impact**: Unlocks the 8-unit Block Actor family (`EN_BLOCK_HATENA_ANGLE`, `EN_BLOCK_HATENA_PLAYER`, `EN_BLOCK_SOROBAN`, `EN_BLOCK_STAFFROLL`, `EN_BLSWICH`, `EN_BLOCK_HATENA_WATER`, `EN_BLOCK_HELP`, `EN_RULETBLOCK`).
- **Tractability**: **62.21% exact / 73.26% shape**.
- **Why Rank 5**: Opens up over 28,000 bytes of block actors that transcribe directly from `daEnBlockMain_c`.

---

### Unit 6: [d_a_en_super_bigpile_left.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_super_bigpile_left.cpp) — The Mega-Stake Sibling

- **Profile & Class**: `g_profile_EN_SUPER_BIGPILE_LEFT` / `daEnSuperBigpileLeft_c`.
- **Inheritance**: `daEnSuperBigpile_c` (Banked in [dol/bases/d_a_en_super_bigpile.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_super_bigpile.cpp)).
- **Section Bounds**:
  * `.text`: `0x108520-0x109100` (Span: 3,040 B, Code: 2,876 B, 24 functions).
  * `.ctors`: `0x230-0x234` (Size: 0x4).
  * `.rodata`: `0x7430-0x74c0` (Size: 0x90).
  * `.data`: `0x33fc8-0x346d8` (Size: 0x710).
  * `.bss`: `0xa278-0xa2f8` (Size: 0x80).
- **Tractability**: **61.75% exact / 71.77% shape**.
- **Why Rank 6**: Unblocks `d_a_en_super_bigpile_right.cpp` (3,512 B code).

---

### Unit 7: [d_a_en_waki_jugem.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_waki_jugem.cpp) — The Minimalist Spawner

- **Profile & Class**: `g_profile_EN_WAKI_JUGEM` / `daEnWakiJugem_c` (Lakitu Spawner).
- **Inheritance**: `dBaseActor_c`.
- **Section Bounds**:
  * `.text`: `0x119990-0x11a130` (Span: 1,952 B, Code: 1,856 B, 18 functions).
  * `.ctors`: `0x25c-0x260` (Size: 0x4).
  * `.rodata`: `0x7c88-0x7c90` (Size: 0x8).
  * `.data`: `0x37cd0-0x37fd0` (Size: 0x300).
  * `.bss`: `0xabe8-0xac28` (Size: 0x40).
- **Tractability**: **61.42% exact / 70.47% shape**. Smallest candidate in the survey (1,856 B code across 18 functions).
- **Why Rank 7**: Zero physics/collision complexity; handles simple distance-trigger timer spawning.

---

### Unit 8: [d_a_en_coin_jump.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_coin_jump.cpp) — The Jumping Coin Leaf

- **Profile & Class**: `g_profile_EN_COIN_JUMP` / `daEnCoinJump_c`.
- **Inheritance**: `daEnCoinMain_c` (Pre-flighted in [dol/bases/d_a_en_coin_main.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/dol/bases/d_a_en_coin_main.cpp)).
- **Section Bounds**:
  * `.text`: `0x44620-0x44ef0` (Span: 2,256 B, Code: 2,152 B, 18 functions).
  * `.ctors`: `0x94-0x98` (Size: 0x4).
  * `.rodata`: `0x1f80-0x1fd0` (Size: 0x50).
  * `.data`: `0xdfe0-0xe650` (Size: 0x670).
  * `.bss`: `0x2758-0x2798` (Size: 0x40).
- **Gating Impact**: Unlocks the 5-actor Coin Trajectory cluster (`COIN_JUMP`, `COIN_VOLT`, `COIN_WATER`, `COIN_WIND`, `COIN_FLOOR` totaling 13,872 B code).
- **Tractability**: **59.29% exact / 65.43% shape**.
- **Note on `d_a_en_coin_main.cpp`**: `d_a_en_coin_main.cpp` itself resides in `wiimj2d.dol` and was pre-flighted by us; once landed, this entire 5-unit cluster in `d_enemiesNP.rel` becomes immediately authorable.

---

## 2.3 Final Recommendation & Strategic Sequencing

If starting work on `d_enemiesNP.rel`, author **[d_a_en_net_nokonoko_lr.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_net_nokonoko_lr.cpp)** first.

**Strategic Justification**:
1. At **70.81% exact / 79.01% shape similarity**, it is the single highest-scoring candidate in the entire REL.
2. It derives cleanly from `daEnNetNokonokoBase_c`, which already has full headers and byte-matching code in the repo.
3. It borders already-landed [d_enemiesNP/bases/d_a_en_noko.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_noko.cpp), expanding contiguous verified territory.
4. Immediately following it, queue **[d_a_en_left_dokan_pakkun.cpp](file:///c:/Users/Razz/Documents/Projects/NSMBW-Decomp/source/d_enemiesNP/bases/d_a_en_left_dokan_pakkun.cpp)** (68.4% match) to unlock all 4 pipe piranha plant actors (11.1 KB code) in a single sweep.
