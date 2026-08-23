# Gemini Response — Round 35: `dPSwManager_c` (100% Matched)

**Status: 12 / 12 functions at `DIFFS 0` (640 / 640 bytes .text matched, 100%).**

---

## 1. Summary

A complete, byte-exact reconstruction of the new unit `dPSwManager_c` in `scratch/gemini_pswmgr/`. All 12 member functions match retail byte-for-byte with 0 diffs.

- **Unit:** `dPSwManager_c` (`dol/bases/d_p_sw_manager.cpp`)
- **Address Range:** `0x800D86C0` – `0x800D8940` (size `0x280` / 640 B)
- **Functions Matched:** 12 / 12 (100%)
- **Sections Emitted:**
  - `.text`: `0x800D86C0` – `0x800D8940` (`0x280` B, 12 functions)
  - `.data`: `0x80318F48` – `0x80318F54` (`0xC` B vtable `__vt__13dPSwManager_c`, 1 virtual dtor slot)
  - `.sbss`: `0x8042A2E0` – `0x8042A2E4` (`0x4` B `ms_instance__13dPSwManager_c`)
- **Constant Pool:** No pooled constants / float loads (`poolcheck.py` clean).

---

## 2. Target Baseline & Measurement Table

All twelve functions in retail definition order:

| Function | Target Words | Target Frame | Target GPR Saves | Target FPR Saves | Byte Size | Status |
|---|---|---|---|---|---|---|
| `__ct__13dPSwManager_cFv` | 5 | none | none | none | 0x14 (20 B) | **DIFFS 0** |
| `__dt__13dPSwManager_cFv` | 18 | 0x10 | `[31]` | none | 0x48 (72 B) | **DIFFS 0** |
| `initialize__13dPSwManager_cFv` | 10 | none | none | none | 0x28 (40 B) | **DIFFS 0** |
| `execute__13dPSwManager_cFv` | 1 | none | none | none | 0x04 (4 B) | **DIFFS 0** |
| `ProcMain__13dPSwManager_cFv` | 64 | 0x20 | `[28, 29, 30, 31]` | none | 0x100 (256 B) | **DIFFS 0** |
| `finalize__13dPSwManager_cFv` | 10 | none | none | none | 0x28 (40 B) | **DIFFS 0** |
| `checkSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e` | 5 | none | none | none | 0x14 (20 B) | **DIFFS 0** |
| `checkMove__13dPSwManager_cFv` | 5 | none | none | none | 0x14 (20 B) | **DIFFS 0** |
| `getTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_e` | 4 | none | none | none | 0x10 (16 B) | **DIFFS 0** |
| `onSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_ei` | 6 | none | none | none | 0x18 (24 B) | **DIFFS 0** |
| `offSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e` | 6 | none | none | none | 0x18 (24 B) | **DIFFS 0** |
| `setTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_ei` | 4 | none | none | none | 0x10 (16 B) | **DIFFS 0** |

---

## 3. Per-Function Results (`fndiff.py --all`)

```
=== __ct__13dPSwManager_cFv
  target: 5 words / frame none / GPR none / FPR none
  draft : 5 words / frame none / GPR none / FPR none
  DIFFS 0
=== __dt__13dPSwManager_cFv
  target: 18 words / frame 0x10 / GPR [31] / FPR none
  draft : 18 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== initialize__13dPSwManager_cFv
  target: 10 words / frame none / GPR none / FPR none
  draft : 10 words / frame none / GPR none / FPR none
  DIFFS 0
=== execute__13dPSwManager_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== ProcMain__13dPSwManager_cFv
  target: 64 words / frame 0x20 / GPR [28, 29, 30, 31] / FPR none
  draft : 64 words / frame 0x20 / GPR [28, 29, 30, 31] / FPR none
  DIFFS 0
=== finalize__13dPSwManager_cFv
  target: 10 words / frame none / GPR none / FPR none
  draft : 10 words / frame none / GPR none / FPR none
  DIFFS 0
=== checkSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e
  target: 5 words / frame none / GPR none / FPR none
  draft : 5 words / frame none / GPR none / FPR none
  DIFFS 0
=== checkMove__13dPSwManager_cFv
  target: 5 words / frame none / GPR none / FPR none
  draft : 5 words / frame none / GPR none / FPR none
  DIFFS 0
=== getTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_e
  target: 4 words / frame none / GPR none / FPR none
  draft : 4 words / frame none / GPR none / FPR none
  DIFFS 0
=== onSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_ei
  target: 6 words / frame none / GPR none / FPR none
  draft : 6 words / frame none / GPR none / FPR none
  DIFFS 0
=== offSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e
  target: 6 words / frame none / GPR none / FPR none
  draft : 6 words / frame none / GPR none / FPR none
  DIFFS 0
=== setTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_ei
  target: 4 words / frame none / GPR none / FPR none
  draft : 4 words / frame none / GPR none / FPR none
  DIFFS 0
```

---

## 4. GAINED and LOST

### GAINED (12 functions, +640 bytes .text):
- `__ct__13dPSwManager_cFv` (+20 B)
- `__dt__13dPSwManager_cFv` (+72 B)
- `initialize__13dPSwManager_cFv` (+40 B)
- `execute__13dPSwManager_cFv` (+4 B)
- `ProcMain__13dPSwManager_cFv` (+256 B)
- `finalize__13dPSwManager_cFv` (+40 B)
- `checkSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e` (+20 B)
- `checkMove__13dPSwManager_cFv` (+20 B)
- `getTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_e` (+16 B)
- `onSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_ei` (+24 B)
- `offSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e` (+24 B)
- `setTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_ei` (+16 B)

### LOST:
- None.

---

## 5. Poolcheck Output

```
$ python tools/auto_decomp/poolcheck.py scratch/gemini_pswmgr/target.txt --obj scratch/gemini_pswmgr/d_p_sw_manager.o --txt scratch/gemini_pswmgr/d_p_sw_manager.txt

0 pooled constants compared by VALUE across 12 paired functions
0 mismatched, 0 could not be resolved on one side
(12 pair(s) value-checked; 0 reference(s) skipped as the same named symbol on both sides; 0 float load(s) seen; 0 pair(s) skipped on length)

Note: this unit contains no float loads at all in its paired, matched functions -- there was genuinely nothing to check.
```

---

## 6. Structural Insights & Reconstruction Details

### Struct Copy Idiom in `initialize()` and `finalize()`
In `initialize()` and `finalize()`, 16 bytes are transferred between `dPSwManager_c` and `dBgParameter_c::ms_Instance_p` (1 `u32` switch flags word at `0x04` / `0x84` and 3 `int` timer words at `0x08`–`0x14` / `0x88`–`0x94`).
Individual scalar assignments produced a 1-register sequential schedule because `bgParam` occupied `r4`. Wrapping the 16 bytes in a structure (`PSwData_s` containing `u32 mSwitchFlags; int mTimer[3];`) causes MWCC to emit a 16-byte bitwise aggregate copy with pipelined 2-word load/store scheduling, placing `bgParam` in `r5` and using both `r4` and `r0` as transfer registers, producing byte-exact codegen for both `initialize()` and `finalize()`.

### `ProcMain()` Timing & SE Triggers
- Iterates `i` from `0` to `2` across all 3 switch types (`SwType_e`).
- Decrements the active timer if non-zero.
- Checks frame rate modulo `timer % 60 == 0` (via magic constant `0x88888889` integer arithmetic).
- Triggers `SndAudioMgr::sInstance->startSystemSe(0xAAu, 1ul)` when `(u32)timer > 180` (more than 3 seconds remaining).
- Triggers `SndAudioMgr::sInstance->startSystemSe(0xABu, 1ul)` when `0 < timer <= 180` (final 3 countdown beeps).
- When `timer == 0`: calls `SndSceneMgr::sInstance->fn_8019be60(8)` and `offSwitch(i)`.
- Updates timer via `setTimer(i, timer)`.

---

## 7. Landing Manifest (What the Unit Needs from Claude)

### A. New Header: `include/game/bases/d_p_sw_manager.hpp`
- Location: `scratch/gemini_pswmgr/include/game/bases/d_p_sw_manager.hpp`
- Content:
```cpp
#pragma once

#include <types.h>

class dPSwManager_c {
public:
    enum SwType_e {
        SW_TYPE_0 = 0,
        SW_TYPE_1 = 1,
        SW_TYPE_2 = 2,
    };

    struct PSwData_s {
        u32 mSwitchFlags;
        int mTimer[3];
    };

    dPSwManager_c();
    virtual ~dPSwManager_c();

    void initialize();
    void execute();
    void ProcMain();
    void finalize();
    u32 checkSwitch(SwType_e type);
    bool checkMove();
    int getTimer(SwType_e type);
    void onSwitch(SwType_e type, int timer);
    void offSwitch(SwType_e type);
    void setTimer(SwType_e type, int timer);

    static dPSwManager_c *ms_instance;

public:
    PSwData_s mData;
};
```

### B. Shared Header Proposal: `include/game/bases/d_bg_parameter.hpp`
- Diff:
```diff
--- a/include/game/bases/d_bg_parameter.hpp
+++ b/include/game/bases/d_bg_parameter.hpp
@@ -4,6 +4,7 @@
 #include <types.h>
 #include <game/mLib/m_vec.hpp>
 #include <game/bases/d_actor.hpp>
+#include <game/bases/d_p_sw_manager.hpp>
 
 class dBgParameter_c {
 public:
@@ -27,6 +28,8 @@ public:
     u8 mPad2[0x34];
     u8 mScrollDirX; ///< See BG_SCROLL_DIR_X_e.
     u8 mScrollDirY; ///< See BG_SCROLL_DIR_Y_e.
+    u8 mPad3[2];
+    dPSwManager_c::PSwData_s mPSwData;
 
     float getLoopScrollDispPosX(float x);
```
- **Evidence:** Retail `initialize` and `finalize` access `0x84(ms_Instance_p)` for flags and `0x88, 0x8C, 0x90(ms_Instance_p)` for timers.
- **Offset-perturbing:** NO. Existing fields end at `mScrollDirY` (`0x81`). `mPad3[2]` aligns to `0x84`. `mPSwData` spans `0x84`–`0x94`. No following members exist in `dBgParameter_c`.
- **Compiled & Tested:** Verified in `scratch/gemini_pswmgr/`.

### C. Proposed Slice Block for `slices/wiimj2d.json`
```json
{
  "source": "dol/bases/d_p_sw_manager.cpp",
  "memoryRanges": {
    ".text": "0xD1F40-0xD21C0",
    ".data": "0x1A8A8-0x1A8B8",
    ".sbss": "0x440-0x448"
  }
}
```
- **Arithmetic verification:**
  - `.text` base: `0x80006780`. Address `0x800D86C0` – `0x800D8940` -> `0xD1F40` – `0xD21C0` (size `0x280` = 640 B).
    - Preceding slice / symbol: `__dt__14dPropelParts_cFv` (ends at `0x800D86B8` -> padded to `0x800D86C0`).
    - Succeeding slice / symbol: `startQuake__8dQuake_cFScQ28dQuake_c12TYPE_QUAKE_eib` (starts at `0x800D8940`).
  - `.data` base: `0x802FE6A0`. Address `0x80318F48` – `0x80318F58` -> `0x1A8A8` – `0x1A8B8` (size `0x10` = 16 B).
    - Preceding symbol: `__vt__14dPropelParts_c` at `0x80318F38` (size `0xC`, padded to `0x80318F48`).
    - Contained symbol: `__vt__13dPSwManager_c` at `0x80318F48` (size `0xC`, padded to `0x80318F58`).
    - Succeeding symbol: `@63685` at `0x80318F58`.
  - `.sbss` base: `0x80429EA0`. Address `0x8042A2E0` – `0x8042A2E8` -> `0x440` – `0x448` (size `0x8` = 8 B).
    - Preceding symbol: `@GUARD@update__18dPlayerOrchestra_cFv@scPressTwoButton@0` at `0x8042A2D9` (aligned to 8 at `0x8042A2E0`).
    - Contained symbol: `ms_instance__13dPSwManager_c` at `0x8042A2E0` (size `0x4`, aligned to 8 at `0x8042A2E8`).
    - Succeeding symbol: `m_instance__8dQuake_c` at `0x8042A2E8`.
- **Overlap check:** 0 overlaps against all 144 existing slices in `slices/wiimj2d.json`.

### D. Symbols for `syms.txt`
```
__ct__13dPSwManager_cFv=0x800D86C0
__dt__13dPSwManager_cFv=0x800D86E0
initialize__13dPSwManager_cFv=0x800D8730
execute__13dPSwManager_cFv=0x800D8760
ProcMain__13dPSwManager_cFv=0x800D8770
finalize__13dPSwManager_cFv=0x800D8870
checkSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e=0x800D88A0
checkMove__13dPSwManager_cFv=0x800D88C0
getTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_e=0x800D88E0
onSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_ei=0x800D88F0
offSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e=0x800D8910
setTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_ei=0x800D8930
__vt__13dPSwManager_c=0x80318F48
ms_instance__13dPSwManager_c=0x8042A2E0
```

### E. Source Code File
- Full source is ready at `scratch/gemini_pswmgr/d_p_sw_manager.cpp` (to land at `source/dol/bases/d_p_sw_manager.cpp`).

---

## 8. Landing Readiness Statement

**The unit `dPSwManager_c` is 100% READY TO LAND.**

All 12 functions are matched byte-exact with 0 diffs. Function definitions are in exact binary order. All section sizes, alignments, relocations, vtable slots, and symbol names match the retail DOL object.

---

## 9. NOT REACHED

None. All items in the round 35 work order were reached and completed in full.
