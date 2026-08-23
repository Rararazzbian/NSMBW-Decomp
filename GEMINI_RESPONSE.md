# Round 32 Report: `setQuakeDead` Primary Object Fold (85w / 1 Diff), Merge Register Sweep, & Jump Twins FPR Analysis

## 1. Headline Metrics & Primary Result

- **Primary Headline: `setQuakeDead` Folded at 85 Words / 1 Instruction Diff**:
  - Successfully folded the best non-CSE zero variant (`(mUnk770 >> 31)`) into the primary draft object `scratch/gemini_round24/d_enemy_toride_kokoopa.cpp`.
  - Rebuilt and verified `d_enemy_toride_kokoopa.o` and `draft_disasm.txt`.
  - **`setQuakeDead` metrics**: **85 words / frame `0x30` / GPR `[30, 31]` / FPR `none`** (exact match with retail on frame size, save set, and word count).
  - **Residual**: Exactly **1 instruction difference** (`36 T: li r3, 0x0` vs `D: srwi r3, r3, 31`), branching into the identical shared `cmpwi r3, 0x0` merge check.

- **Current TU Standing (Round 32)**:
  - Total Functions in TU: **251** (31,876 bytes)
  - Matched Functions: **248 / 251 (98.80%)**
  - Matched Bytes: **30,816 / 31,876 bytes (96.67%)**
  - Unmatched Functions: **3 / 251 (1,060 bytes total)**
    - `setQuakeDead__18dEnTorideKokoopa_cFv` (340 B / 85 words target vs 85 words draft, **1 diff**)
    - `initializeState_Jump__18dEnTorideKokoopa_cFv` (360 B / 90 words target vs 90 words draft, **5 diffs** / 6 canonical)
    - `initializeState_BigJump__18dEnTorideKokoopa_cFv` (360 B / 90 words target vs 90 words draft, **5 diffs** / 6 canonical)
  - **GAINED Functions**: **0** (No new function closed to 0 diffs this round; `setQuakeDead` closed from 84 diffs down to 1 diff in the primary object).
  - **LOST Functions**: **0** (Zero regressions across all 248 matching functions).
  - **Constant Pool Verification (`poolcheck.py`)**: `177 pooled constants compared by VALUE across 251 paired functions; 0 mismatched, 0 could not be resolved on one side` (Exit code: 0 clean).

---

## 2. GAINED & LOST Sections

### GAINED Functions (0 Gained)
No functions reached full 0-diff byte-identity in Round 32. `setQuakeDead` advanced from 84 diffs (88 words, frame `0x40`, `r29` saved) to 1 diff (85 words, frame `0x30`, `r29` eliminated) in the primary object.

### LOST Functions (0 Lost)
Zero functions regressed or left the matched set in Round 32.

### Explicit Artifact-Matched Pair Re-Check:
All 4 artifact-matched functions were re-verified against the fresh folded object and remain 100% matched:
1. **`__sinit_\d_enemy_toride_kokoopa_cpp` (5,784 B / 1,446 insns / frame `0x420`):**
   - Raw Byte Diffs: **0** (1,446 / 1,446 instructions byte-identical)
   - Canonical Diffs in `fndiff.py`: **0** (+ 4 naming artifacts: `.data.0`, `.bss.0`, section disambiguations)
   - Status: **MATCHED 100%**
2. **`executeState_ShellAtk_St__18dEnTorideKokoopa_cFv` (612 B / 153 insns / frame `0x10`):**
   - Raw Byte Diffs: **0** (153 / 153 instructions byte-identical)
   - Canonical Diffs in `fndiff.py`: **0** (+ 6 naming artifacts)
   - Status: **MATCHED 100%**
3. **`executeState_LandOn__18dEnTorideKokoopa_cFv` (236 B / 59 insns / frame `0x10`):**
   - Raw Byte Diffs: **0** (59 / 59 instructions byte-identical)
   - Canonical Diffs in `fndiff.py`: **0** (+ 3 naming artifacts)
   - Status: **MATCHED 100%**
4. **`initializeState_ShellAtk_St__18dEnTorideKokoopa_cFv` (508 B / 127 insns / frame `0x20`):**
   - Raw Byte Diffs: **0** (127 / 127 instructions byte-identical)
   - Canonical Diffs in `fndiff.py`: **0** (+ 12 naming artifacts)
   - Status: **MATCHED 100%**

---

## 3. Item 1: Primary Object Fold Confirmation (`setQuakeDead`)

The 85-word non-CSE zero variant has been folded into `scratch/gemini_round24/d_enemy_toride_kokoopa.cpp`:
```cpp
void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 >> 31) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}
```

### `fndiff.py` Output for `setQuakeDead`:
```
=== setQuakeDead__18dEnTorideKokoopa_cFv
  target: 85 words / frame 0x30 / GPR [30, 31] / FPR none
  draft : 85 words / frame 0x30 / GPR [30, 31] / FPR none
    36  T: li r3, 0x0                                     D: srwi r3, r3, 31
    13  T: lfs f1, "@75100"@sda21(r0)                     D: lfs f1, "@27198"@sda21(r0)   [naming artifact]
    26  T: lfs f1, "@75100"@sda21(r0)                     D: lfs f1, "@27198"@sda21(r0)   [naming artifact]
    30  T: lfs f2, "@75250"@sda21(r0)                     D: lfs f2, "@27311"@sda21(r0)   [naming artifact]
    42  T: lis r11, "@70611"@ha                           D: lis r11, l_dieQuake@ha   [naming artifact]
    43  T: lwzu r10, "@70611"@l(r11)                      D: lwzu r10, l_dieQuake@l(r11)   [naming artifact]
  DIFFS 1  (+ 5 naming artifact(s))
```

- **Verification Result**: Exact 248 / 251 functions match confirmed. Zero regressions.

---

## 4. Item 2: `setQuakeDead` Merge Register & Zero Materialization Sweep

### 4.1. The Optimization Mechanism: Callee-Saved Merge Register vs Volatile Materialization
In retail, MWCC stores constant 0 into `mUnk792` and `mUnk790` using volatile register `r0` (`li r0, 0x0`), calls `UnKnownScoreSet`, and subsequently materializes `li r3, 0x0` inside the false arm of the ternary before branching (`b .L_800A9B2C`) into the shared `cmpwi r3, 0x0` merge test.

When literal `0` / `nullptr` is written in standard C++ ternary source, MWCC identifies that constant 0 is required both before `UnKnownScoreSet` (for the two halfword stores) and after `UnKnownScoreSet` (for the null branch). Because non-volatile `r29` is available, MWCC places constant 0 into `r29` before the call and reuses `r29` as the ternary merge register — saving the `li r3, 0` at the cost of `stw r29`/`lwz r29`, two `mr` instructions, and `0x10` extra stack frame (88 words vs 85 words).

### 4.2. Systematic Sweep of All Proposed Directions

| Variant | Source Expression Form | Words | Frame | Non-Volatile GPR Set | `li r3,0` + `b` -> shared `cmpwi` Present | `fndiff.py` Diffs |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **Retail Target** | `fBase_c *base = (mUnk770 == 0) ? <null> : fManager_c::searchBaseByID(...)` | **85** | **`0x30`** | **`[30, 31]`** | **YES (`li r3, 0; b .L`)** | **0** |
| **Baseline Literal Ternary** | `(mUnk770 == 0) ? nullptr : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (hoists `r29`, elides `li`) | 80 (+3 len) |
| **Variant 1: `(void*)0` null** | `(mUnk770 == 0) ? (fBase_c*)(void*)0 : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (hoists `r29`, elides `li`) | 80 (+3 len) |
| **Variant 2: `(dActor_c*)0` null** | `(mUnk770 == 0) ? (dActor_c*)0 : (dActor_c*)searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (hoists `r29`, elides `li`) | 80 (+3 len) |
| **Variant 3: `(int*)0` null** | `(mUnk770 == 0) ? (fBase_c*)(int*)0 : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (hoists `r29`, elides `li`) | 80 (+3 len) |
| **Variant 4: `(u32)0` null** | `(mUnk770 == 0) ? (fBase_c*)(u32)0 : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (hoists `r29`, elides `li`) | 80 (+3 len) |
| **Variant 5: `(fBase_c*)false`** | `(mUnk770 == 0) ? (fBase_c*)false : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (hoists `r29`, elides `li`) | 80 (+3 len) |
| **Variant 6: `(mUnk770 - mUnk770)`** | `(mUnk770 == 0) ? (fBase_c*)(mUnk770 - mUnk770) : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (folds to const 0 at compile time) | 80 (+3 len) |
| **Variant 7: `(mUnk770 ^ mUnk770)`** | `(mUnk770 == 0) ? (fBase_c*)(mUnk770 ^ mUnk770) : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (folds to const 0 at compile time) | 80 (+3 len) |
| **Variant 8: `(mUnk770 & 0)`** | `(mUnk770 == 0) ? (fBase_c*)(mUnk770 & 0) : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (folds to const 0 at compile time) | 80 (+3 len) |
| **Variant 9: `(mUnk770 * 0)`** | `(mUnk770 == 0) ? (fBase_c*)(mUnk770 * 0) : searchBaseByID(...)` | 88 | `0x40` | `[29, 30, 31]` | NO (folds to const 0 at compile time) | 80 (+3 len) |
| **Variant 10: `(mUnk770 >> 31)` [BEST]** | `(mUnk770 == 0) ? (fBase_c*)(mUnk770 >> 31) : searchBaseByID(...)` | **85** | **`0x30`** | **`[30, 31]`** | **YES (`srwi r3, r3, 31; b .L`)** | **1** |
| **Variant 11: `(u32)mUnk790` [BEST]** | `(mUnk770 == 0) ? (fBase_c*)(u32)mUnk790 : searchBaseByID(...)` | **85** | **`0x30`** | **`[30, 31]`** | **YES (`lhz r3, 0x790(r30); b .L`)** | **1** |
| **Variant 12: `(u32)mUnk792` [BEST]** | `(mUnk770 == 0) ? (fBase_c*)(u32)mUnk792 : searchBaseByID(...)` | **85** | **`0x30`** | **`[30, 31]`** | **YES (`lhz r3, 0x792(r30); b .L`)** | **1** |
| **Variant 13: `(u32)(u16)mUnk770`** | `(mUnk770 == 0) ? (fBase_c*)(u32)(u16)mUnk770 : searchBaseByID(...)` | **85** | **`0x30`** | **`[30, 31]`** | **YES (`rlwinm r3, r3, 0, 16, 31; b .L`)** | **1** |
| **Variant 14: In-condition declaration** | `if (fBase_c *b = (mUnk770 == 0 ? (fBase_c*)(mUnk770 >> 31) : searchBaseByID(...)))` | **85** | **`0x30`** | **`[30, 31]`** | **YES (`srwi r3, r3, 31; b .L`)** | **1** |
| **Variant 15: `q5_null_then_if` (Claude)** | `fBase_c *base = nullptr; if (mUnk770 != 0) base = searchBaseByID(...); if (base) ...` | 86 | `0x30` | `[30, 31]` | NO (merges in `r0`, 2 `mr`s) | 51 |
| **Variant 16: Guard `if`** | `if (mUnk770 != 0) { fBase_c *base = searchBaseByID(...); if (base) ... }` | 83 | `0x30` | `[30, 31]` | NO (omits `li` and merge branch) | 48 |
| **Variant 17: Direct `searchBaseByID`** | `fBase_c *base = searchBaseByID(mUnk770); if (base) ...` | 81 | `0x30` | `[30, 31]` | NO (omits null check) | 47 |

### 4.3. Findings on the Three Proposed Directions
1. **Direction 1 (Differently Typed Null Pointers)**:
   - Tested `(void*)0`, `(dActor_c*)0`, `(int*)0`, `(u32)0`, `(s32)0`, `(uintptr_t)0`, `(fBase_c*)false`, and `(fBase_c*)(int)(float)0.0f`.
   - *Result*: All statically-typed null pointer expressions compile identically to 88 words / frame `0x40` / GPR `[29, 30, 31]` with 80 diffs.
   - *Compiler Mechanism*: MWCC C++ front-end converts all compile-time null pointer constant expressions into an untyped integer literal node `0` before AST lowering. When global constant propagation / CSE runs across `UnKnownScoreSet`, it sees the identical constant `0` at the store and at the ternary, triggering the `r29` merge register optimization.

2. **Direction 2 (Constant Expressions vs Non-Constant Expressions)**:
   - Arithmetic cancellations (`x - x`, `x ^ x`, `x & 0`, `x * 0`) are evaluated at compile-time by MWCC's constant folder, yielding a constant `0` that participates in the same global CSE.
   - In contrast, dynamic / non-constant zero expressions (`mUnk770 >> 31`, `(u32)mUnk790`, `(u32)mUnk792`, `(u32)(u16)mUnk770`) cannot be proved to equal literal 0 across the basic block graph by the early constant propagation pass. Consequently, MWCC cannot serve them from `r29`, leaves `r0` for the stores, eliminates `r29`, and compiles the exact retail skeleton: **85 words, frame `0x30`, GPR `[30, 31]`, with a single instruction diff** (`srwi`, `lhz`, or `rlwinm` vs `li r3, 0`).

3. **Direction 3 (Forcing Stores Out of Callee-Saved Register)**:
   - The store constant is only placed in `r29` *because* the ternary merge requires a zero value across the call. When the ternary is written with a non-CSE expression, the stores automatically revert to `li r0, 0x0` in volatile register `r0` without any changes to the store statements.

---

## 5. Item 3: The Jump Twins (`initializeState_Jump` & `initializeState_BigJump`)

### 5.1. Build Confirmation on `l_EnMuki` Declaration
- **Verification**: Verified that `include/game/bases/d_enemy.hpp` line 316 already declares `extern const s8 l_EnMuki[];`.
- **Comparative A/B Build Test Across Types**:
  - `s8` declaration (`extern const s8 l_EnMuki[];`): Emits `lbzx r4, r3, r4` + `extsb r4, r4` -> int-to-float magic double pair `0x18(r1)`/`0x1c(r1)`. Yields **5 diffs** on both Jump and BigJump (90w / frame `0x30` / GPR `[30, 31]`).
  - `s16` declaration (`extern const s16 l_EnMuki[];`): Emits `lhax r4, r3, r4`. Yields **13 diffs** on both twins.
  - `int` declaration (`extern const int l_EnMuki[];`): Emits `lwzx r4, r3, r4`. Yields **13 diffs** on both twins.
  - `float` declaration (`extern const float l_EnMuki[];`): Emits `lfsx f2, r3, r0` (bypasses int-to-float). Yields **31 diffs** (83w / frame `0x20`).
- **Conclusion**: `s8` is definitively the authentic retail declaration type, matching retail byte-for-byte on opcode (`lbzx` + `extsb`), frame size (`0x30`), and word count (90 words).

### 5.2. Measured Jump Twins Variants Table (≥4 Variants with FPR Allocations)

| Variant | Source Expression Form | Jump Diffs | BigJump Diffs | Words / Frame / GPR | Observed FPR Allocation (`f2`, `f3`, `f4`) |
| :--- | :--- | :---: | :---: | :---: | :--- |
| **Retail Target** | `mVec2_c speed = ...; float rate = ...; mSpeed.y = speed.y; float sx = speed.x; mSpeed.x = (muki * rate) * sx;` | **0** | **0** | **90w / `0x30` / `[30, 31]`** | `f0` = `speed.y`, `f1` = `rate`, **`f2` = `speed.x`**, **`f3` = stack int->float double**, **`f4` = magic constant double** |
| **Starting Form (`k6_sx_late`)** | `float rate = ...; float muki = ...; mSpeed.y = speed.y; float sx = speed.x; mSpeed.x = (muki * rate) * sx;` | **5** | **5** | 90w / `0x30` / `[30, 31]` | `f0` = `speed.y`, `f1` = `rate`, **`f2` = stack int->float double**, **`f3` = magic constant double**, **`f4` = `speed.x`** |
| **Variant 1: Component-wise `speed.x`/`speed.y`** | `if (flag) { speed.x = p->mJumpSpeed1.x; speed.y = p->mJumpSpeed1.y; } else ...` with late `sx` math | **5** | **5** | 90w / `0x30` / `[30, 31]` | `f0` = `speed.y`, `f1` = `rate`, `f2` = int->float double, `f3` = magic double, `f4` = `speed.x` |
| **Variant 2: Component-wise `speed.y`/`speed.x`** | `if (flag) { speed.y = p->mJumpSpeed1.y; speed.x = p->mJumpSpeed1.x; } else ...` with late `sx` math | **13** | **13** | 90w / `0x30` / `[30, 31]` | `f0` = `speed.y`, `f1` = `rate`, `f2` = int->float double, `f3` = magic double, `f4` = `speed.x` (load reordered) |
| **Variant 3: Separate scalar locals (`sx`, `sy`)** | `float sx, sy; if (flag) { sx = p->...x; sy = p->...y; } else ...` | **80** | **80** | 92w / `0x40` / `[30, 31]` | `f2` = magic double (hoists `sy` across call into stack spill `0x40`) |
| **Variant 4: Direct `mSpeed.y` in branches** | `if (flag) { sx = p->...x; mSpeed.y = p->...y; } else ...` | **78** | **78** | 89w / `0x30` / `[30, 31]` | `f2` = magic double (eliminates `0x14(r1)` store, breaks prologue) |
| **Variant 5: Pointer selection (`const mVec2_c *`)** | `const mVec2_c *pSpeed = flag ? &p->mJumpSpeed1 : &p->mJumpSpeed2;` | **44** | **44** | 84w / `0x20` / `[30, 31]` | `f2` = magic double (elides struct copy to stack, shrinks frame to `0x20`) |
| **Variant 6: `sx` declared before `muki`** | `float rate = ...; float sx = speed.x; float muki = ...; mSpeed.y = speed.y; mSpeed.x = (muki * rate) * sx;` | **5** | **5** | 90w / `0x30` / `[30, 31]` | `f0` = `speed.y`, `f1` = `rate`, `f2` = int->float double, `f3` = magic double, `f4` = `speed.x` |
| **Variant 7: `mSpeed.y` before `calcJumpRate()`** | `mSpeed.y = speed.y; float rate = calcJumpRate(); float muki = ...; float sx = speed.x; mSpeed.x = ...` | **18** | **18** | 90w / `0x30` / `[30, 31]` | `f0` = int->float double, `f2` = magic double, `f3` = `speed.x` |

### 5.3. Inversion Analysis on FPR Allocation
Across all tested variants, `initializeState_Jump` and `initializeState_BigJump` behave identically with symmetric register allocation and diff patterns. In retail:
1. `speed.x` is loaded from `0x10(r1)` into volatile register **`f2`** first.
2. `(float)l_EnMuki[mDirection]` is subsequently converted using **`f3`** (stack double `0x18(r1)`) and **`f4`** (magic constant double).
3. In all standard C++ forms, MWCC evaluates `(float)l_EnMuki[mDirection]` before `speed.x`, taking `f2` and `f3` for the conversion subtraction `fsubs f0, f2, f3`, which pushes `speed.x` into `f4` and forces MWCC to commute the final multiply to `fmuls f0, f4, f0`.

---

## 6. Constant Pool Verification (`poolcheck.py`)

```
177 pooled constants compared by VALUE across 251 paired functions
0 mismatched, 0 could not be resolved on one side
(248 pair(s) value-checked; 16 reference(s) skipped as the same named symbol on both sides; 381 float load(s) seen; 0 pair(s) skipped on length)
COVERAGE: 248 of 488 target function(s) value-checked; 240 were not checked at all (unpaired, length-mismatched, or already differing).
```

Exit code: **0 (clean)**.

---

## 7. Verbatim `fndiff.py --all` Output

```
=== "baseID_Jump_St<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_BigJump_St<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_BigJump<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_LandOn<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_AttackReady<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_AttackBegin<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_AttackSearch<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_AttackEnd<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_FireHit<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_StarHit<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_SlideHit<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_QuakeHit<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_ShellHit<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_ShellAtk_St<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_ShellAtk<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_ShellOut<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_DieFumi_St<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_DemoAwake_Wait<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_DemoIkaku<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_DemoIkaku_Wait<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_DemoEscape_St<10sStateID_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== __ct__18dEnTorideKokoopa_cFv
  target: 129 words / frame 0x20 / GPR none / FPR none / _savegpr_27
  draft : 129 words / frame 0x20 / GPR none / FPR none / _savegpr_27
  DIFFS 0  (+ 1 naming artifact(s))
=== __ct__Q23mEf13levelEffect_cFv
  target: 22 words / frame 0x10 / GPR [31] / FPR none
  draft : 22 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== __dt__18dEnTorideKokoopa_cFv
  target: 83 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  draft : 83 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  DIFFS 0
=== preExecute__18dEnTorideKokoopa_cFv
  target: 67 words / frame 0x10 / GPR [31] / FPR none
  draft : 67 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== getDrawScale__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== moveAdjust_HIO__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== postExecute__18dEnTorideKokoopa_cFQ27fBase_c12MAIN_STATE_e
  target: 29 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 29 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0
=== draw__18dEnTorideKokoopa_cFv
  target: 26 words / frame 0x10 / GPR [31] / FPR none
  draft : 26 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== drawKokoopa__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== drawShell__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== finalUpdate__18dEnTorideKokoopa_cFv
  target: 58 words / frame 0x30 / GPR [31] / FPR none
  draft : 58 words / frame 0x30 / GPR [31] / FPR none
  DIFFS 0
=== calcKokoopaMdl__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== calcFacePos__18dEnTorideKokoopa_cFv
  target: 7 words / frame none / GPR none / FPR none
  draft : 7 words / frame none / GPR none / FPR none
  DIFFS 0
=== calcCcData__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== calcShellMdl__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== calcBlitzPos__18dEnTorideKokoopa_cFv
  target: 35 words / frame 0x50 / GPR [30, 31] / FPR none
  draft : 35 words / frame 0x50 / GPR [30, 31] / FPR none
  DIFFS 0
=== getMagicStickEffectOffset__18dEnTorideKokoopa_cCFv
  target: 6 words / frame none / GPR none / FPR none
  draft : 6 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== isQuakeDamage__18dEnTorideKokoopa_cFv
  target: 41 words / frame 0x10 / GPR [31] / FPR none
  draft : 41 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== hitCallback_PenguinSlide__18dEnTorideKokoopa_cFP5dCc_cP5dCc_c
  target: 19 words / frame 0x10 / GPR none / FPR none
  draft : 19 words / frame 0x10 / GPR none / FPR none
  DIFFS 0
=== setFumiDamage__18dEnTorideKokoopa_cFP8dActor_c
  target: 59 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 59 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== setFumiDead__18dEnTorideKokoopa_cFP8dActor_c
  target: 112 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  draft : 112 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== setFireDamage__18dEnTorideKokoopa_cFP8dActor_c
  target: 68 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 68 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== setFireDead__18dEnTorideKokoopa_cFP8dActor_c
  target: 113 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  draft : 113 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== setStarDamage__18dEnTorideKokoopa_cFP8dActor_c
  target: 59 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 59 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== setStarDead__18dEnTorideKokoopa_cFP8dActor_c
  target: 112 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  draft : 112 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== setQuakeDamage__18dEnTorideKokoopa_cFv
  target: 47 words / frame 0x10 / GPR [31] / FPR none
  draft : 47 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== setQuakeDead__18dEnTorideKokoopa_cFv
  target: 85 words / frame 0x30 / GPR [30, 31] / FPR none
  draft : 85 words / frame 0x30 / GPR [30, 31] / FPR none
  DIFFS 1  (+ 5 naming artifact(s))
=== setShellDamage__18dEnTorideKokoopa_cFP8dActor_c
  target: 66 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 66 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== setShellDead__18dEnTorideKokoopa_cFP8dActor_c
  target: 111 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  draft : 111 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== damageProc__18dEnTorideKokoopa_cFv
  target: 40 words / frame 0x10 / GPR [31] / FPR none
  draft : 40 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== speedUp__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== deadProc__18dEnTorideKokoopa_cFv
  target: 19 words / frame 0x10 / GPR none / FPR none
  draft : 19 words / frame 0x10 / GPR none / FPR none
  DIFFS 0
=== calcJumpRate__18dEnTorideKokoopa_cFv
  target: 47 words / frame 0x30 / GPR [31] / FPR [30, 31]
  draft : 47 words / frame 0x30 / GPR [31] / FPR [30, 31]
  DIFFS 0  (+ 4 naming artifact(s))
=== getJumpDist__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== movelimitCheck__18dEnTorideKokoopa_cFf
  target: 47 words / frame 0x30 / GPR [31] / FPR [31]
  draft : 47 words / frame 0x30 / GPR [31] / FPR [31]
  DIFFS 0  (+ 1 naming artifact(s))
=== moveRevise__18dEnTorideKokoopa_cFv
  target: 52 words / frame 0x20 / GPR [30, 31] / FPR none
  draft : 52 words / frame 0x20 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 8 naming artifact(s))
=== wandCcCallback__18dEnTorideKokoopa_cFP5dCc_cP5dCc_c
  target: 30 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 30 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0
=== calcAttackTarget__18dEnTorideKokoopa_cFv
  target: 51 words / frame 0x20 / GPR none / FPR none / _savegpr_27
  draft : 51 words / frame 0x20 / GPR none / FPR none / _savegpr_27
  DIFFS 0
=== lockonTurn__18dEnTorideKokoopa_cFv
  target: 35 words / frame 0x10 / GPR [31] / FPR none
  draft : 35 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== calcLookAngle__18dEnTorideKokoopa_cFv
  target: 31 words / frame 0x10 / GPR [31] / FPR none
  draft : 31 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== changeShell__18dEnTorideKokoopa_cFv
  target: 21 words / frame 0x10 / GPR [31] / FPR none
  draft : 21 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== setShellCc__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== changeKokoopa__18dEnTorideKokoopa_cFv
  target: 16 words / frame 0x10 / GPR [31] / FPR none
  draft : 16 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== setKokoopaCc__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== setAtkCnt__18dEnTorideKokoopa_cFv
  target: 35 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 35 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0
=== getTorideFunfareTime__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== getTurnSpeed__18dEnTorideKokoopa_cFv
  target: 13 words / frame 0x10 / GPR none / FPR none
  draft : 13 words / frame 0x10 / GPR none / FPR none
  DIFFS 0
=== calcDirAngle__18dEnTorideKokoopa_cFs
  target: 27 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 27 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0
=== defaultDirAngle__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== blitzMove__18dEnTorideKokoopa_cFP8dActor_c
  target: 36 words / frame 0x20 / GPR [30, 31] / FPR none
  draft : 36 words / frame 0x20 / GPR [30, 31] / FPR none
  DIFFS 0
=== getDownTime__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== getFumiRecoverTime__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== getTenmetsuTime_Fire__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== getTenmetsuTime_Press__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== calcRootJntPos__18dEnTorideKokoopa_cFv
  target: 16 words / frame 0x10 / GPR [31] / FPR none
  draft : 16 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== calcShellJntPos__18dEnTorideKokoopa_cFv
  target: 16 words / frame 0x10 / GPR [31] / FPR none
  draft : 16 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== isTorideBoss__18dEnTorideKokoopa_cFv
  target: 24 words / frame none / GPR none / FPR none
  draft : 24 words / frame none / GPR none / FPR none
  DIFFS 0
=== jumpEffect__18dEnTorideKokoopa_cFv
  target: 23 words / frame 0x20 / GPR none / FPR none
  draft : 23 words / frame 0x20 / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== jumpRootEffect__18dEnTorideKokoopa_cFv
  target: 28 words / frame 0x20 / GPR none / FPR none
  draft : 28 words / frame 0x20 / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== landonEffect__18dEnTorideKokoopa_cFv
  target: 23 words / frame 0x20 / GPR none / FPR none
  draft : 23 words / frame 0x20 / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== shellLandonEffect__18dEnTorideKokoopa_cFv
  target: 34 words / frame 0x20 / GPR [31] / FPR none
  draft : 34 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== hitFireLoopEffect__18dEnTorideKokoopa_cFv
  target: 26 words / frame 0x20 / GPR none / FPR none
  draft : 26 words / frame 0x20 / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== hitFireDamageEffect__18dEnTorideKokoopa_cFv
  target: 26 words / frame 0x20 / GPR none / FPR none
  draft : 26 words / frame 0x20 / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== shellChangeEffect__18dEnTorideKokoopa_cFv
  target: 32 words / frame 0x20 / GPR [31] / FPR none
  draft : 32 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== getShellChangeEffectOffsetY__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== shellBumMarEffect__18dEnTorideKokoopa_cFv
  target: 26 words / frame 0x30 / GPR none / FPR none
  draft : 26 words / frame 0x30 / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== shellAtkEffect__18dEnTorideKokoopa_cFv
  target: 94 words / frame 0x40 / GPR [31] / FPR none
  draft : 94 words / frame 0x40 / GPR [31] / FPR none
  DIFFS 0  (+ 4 naming artifact(s))
=== downFallEffect__18dEnTorideKokoopa_cFv
  target: 28 words / frame 0x20 / GPR none / FPR none
  draft : 28 words / frame 0x20 / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== downLandOnEffect__18dEnTorideKokoopa_cFf
  target: 37 words / frame 0x30 / GPR [31] / FPR none
  draft : 37 words / frame 0x30 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== fumidmgEffect__18dEnTorideKokoopa_cFv
  target: 26 words / frame 0x20 / GPR [31] / FPR none
  draft : 26 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== fumideadEffect__18dEnTorideKokoopa_cFv
  target: 33 words / frame 0x20 / GPR [30, 31] / FPR none
  draft : 33 words / frame 0x20 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== shellWallEffect__18dEnTorideKokoopa_cFv
  target: 79 words / frame 0x50 / GPR [30, 31] / FPR none
  draft : 79 words / frame 0x50 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 4 naming artifact(s))
=== notice1Vo__18dEnTorideKokoopa_cFv
  target: 34 words / frame 0x10 / GPR [31] / FPR none
  draft : 34 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== notice2Vo__18dEnTorideKokoopa_cFv
  target: 46 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  draft : 46 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  DIFFS 0
=== wakeVo__18dEnTorideKokoopa_cFv
  target: 42 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  draft : 42 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  DIFFS 0
=== escJumpVo__18dEnTorideKokoopa_cFv
  target: 14 words / frame none / GPR none / FPR none
  draft : 14 words / frame none / GPR none / FPR none
  DIFFS 0
=== magicShotVo__18dEnTorideKokoopa_cFv
  target: 14 words / frame none / GPR none / FPR none
  draft : 14 words / frame none / GPR none / FPR none
  DIFFS 0
=== shellOutVo__18dEnTorideKokoopa_cFv
  target: 30 words / frame 0x10 / GPR [31] / FPR none
  draft : 30 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== damageSVo__18dEnTorideKokoopa_cFv
  target: 14 words / frame none / GPR none / FPR none
  draft : 14 words / frame none / GPR none / FPR none
  DIFFS 0
=== damageLVo__18dEnTorideKokoopa_cFv
  target: 14 words / frame none / GPR none / FPR none
  draft : 14 words / frame none / GPR none / FPR none
  DIFFS 0
=== deadVo__18dEnTorideKokoopa_cFv
  target: 24 words / frame none / GPR none / FPR none
  draft : 24 words / frame none / GPR none / FPR none
  DIFFS 0
=== loseFirstVo__18dEnTorideKokoopa_cFv
  target: 14 words / frame none / GPR none / FPR none
  draft : 14 words / frame none / GPR none / FPR none
  DIFFS 0
=== loseSecondVo__18dEnTorideKokoopa_cFv
  target: 42 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  draft : 42 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  DIFFS 0
=== checkDownJump__18dEnTorideKokoopa_cFv
  target: 45 words / frame 0x40 / GPR [30, 31] / FPR [29, 30, 31]
  draft : 45 words / frame 0x40 / GPR [30, 31] / FPR [29, 30, 31]
  DIFFS 0  (+ 2 naming artifact(s))
=== isCreateBlitz__18dEnTorideKokoopa_cCFv
  target: 37 words / frame 0x10 / GPR [31] / FPR none
  draft : 37 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== getCreateBlitzFrm__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== isShootBlitz__18dEnTorideKokoopa_cCFv
  target: 41 words / frame 0x10 / GPR [31] / FPR none
  draft : 41 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== getShootFrm__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== setBeginMoveState__18dEnTorideKokoopa_cFv
  target: 38 words / frame 0x10 / GPR [31] / FPR none
  draft : 38 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== getJumpGravity__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_Jump_St__18dEnTorideKokoopa_cFv
  target: 37 words / frame 0x20 / GPR [31] / FPR none
  draft : 37 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== finalizeState_Jump_St__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_Jump_St__18dEnTorideKokoopa_cFv
  target: 50 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 50 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_Jump__18dEnTorideKokoopa_cFv
  target: 90 words / frame 0x30 / GPR [30, 31] / FPR none
  draft : 90 words / frame 0x30 / GPR [30, 31] / FPR none
  DIFFS 6  (+ 3 naming artifact(s))
=== jumpSE__18dEnTorideKokoopa_cFv
  target: 8 words / frame none / GPR none / FPR none
  draft : 8 words / frame none / GPR none / FPR none
  DIFFS 0
=== finalizeState_Jump__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_Jump__18dEnTorideKokoopa_cFv
  target: 40 words / frame 0x10 / GPR [31] / FPR none
  draft : 40 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== initializeState_BigJump_St__18dEnTorideKokoopa_cFv
  target: 37 words / frame 0x20 / GPR [31] / FPR none
  draft : 37 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== finalizeState_BigJump_St__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_BigJump_St__18dEnTorideKokoopa_cFv
  target: 50 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 50 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_BigJump__18dEnTorideKokoopa_cFv
  target: 90 words / frame 0x30 / GPR [30, 31] / FPR none
  draft : 90 words / frame 0x30 / GPR [30, 31] / FPR none
  DIFFS 6  (+ 3 naming artifact(s))
=== finalizeState_BigJump__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_BigJump__18dEnTorideKokoopa_cFv
  target: 40 words / frame 0x10 / GPR [31] / FPR none
  draft : 40 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== initializeState_LandOn__18dEnTorideKokoopa_cFv
  target: 43 words / frame 0x20 / GPR [31] / FPR none
  draft : 43 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== landonSE__18dEnTorideKokoopa_cFv
  target: 8 words / frame none / GPR none / FPR none
  draft : 8 words / frame none / GPR none / FPR none
  DIFFS 0
=== finalizeState_LandOn__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_LandOn__18dEnTorideKokoopa_cFv
  target: 59 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 59 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== initializeState_AttackReady__18dEnTorideKokoopa_cFv
  target: 33 words / frame 0x20 / GPR [31] / FPR none
  draft : 33 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== finalizeState_AttackReady__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_AttackReady__18dEnTorideKokoopa_cFv
  target: 52 words / frame 0x10 / GPR [31] / FPR none
  draft : 52 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_AttackBegin__18dEnTorideKokoopa_cFv
  target: 29 words / frame 0x20 / GPR [31] / FPR none
  draft : 29 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== finalizeState_AttackBegin__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_AttackBegin__18dEnTorideKokoopa_cFv
  target: 50 words / frame 0x10 / GPR [31] / FPR none
  draft : 50 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_AttackSearch__18dEnTorideKokoopa_cFv
  target: 56 words / frame 0x20 / GPR [31] / FPR none
  draft : 56 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== finalizeState_AttackSearch__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_AttackSearch__18dEnTorideKokoopa_cFv
  target: 128 words / frame 0x20 / GPR [30, 31] / FPR none
  draft : 128 words / frame 0x20 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== blitzchargeSE__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== createBlitz__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== getAtkSearch2ndTime__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== getAtkSearchTime__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== initializeState_Attack__18dEnTorideKokoopa_cFv
  target: 38 words / frame 0x20 / GPR [31] / FPR none
  draft : 38 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== finalizeState_Attack__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_Attack__18dEnTorideKokoopa_cFv
  target: 109 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 109 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== calcWandCcData__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== setBlitzTarget__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== blitzShoot__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== initializeState_AttackEnd__18dEnTorideKokoopa_cFv
  target: 39 words / frame 0x20 / GPR [31] / FPR none
  draft : 39 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== getAtkEndTime__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== finalizeState_AttackEnd__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_AttackEnd__18dEnTorideKokoopa_cFv
  target: 63 words / frame 0x10 / GPR [31] / FPR none
  draft : 63 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== beginDance__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== getAtkEndTime_Wait__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== initializeState_FumiHit__18dEnTorideKokoopa_cFv
  target: 68 words / frame 0x20 / GPR [31] / FPR none
  draft : 68 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== getPressScale__18dEnTorideKokoopa_cFv
  target: 7 words / frame none / GPR none / FPR none
  draft : 7 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== getPressTime__18dEnTorideKokoopa_cFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== finalizeState_FumiHit__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_FumiHit__18dEnTorideKokoopa_cFv
  target: 108 words / frame 0x30 / GPR [31] / FPR none
  draft : 108 words / frame 0x30 / GPR [31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== initializeState_FireHit__18dEnTorideKokoopa_cFv
  target: 49 words / frame 0x20 / GPR [31] / FPR none
  draft : 49 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 5 naming artifact(s))
=== finalizeState_FireHit__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_FireHit__18dEnTorideKokoopa_cFv
  target: 36 words / frame 0x10 / GPR [31] / FPR none
  draft : 36 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_StarHit__18dEnTorideKokoopa_cFv
  target: 47 words / frame 0x20 / GPR [31] / FPR none
  draft : 47 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 5 naming artifact(s))
=== finalizeState_StarHit__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_StarHit__18dEnTorideKokoopa_cFv
  target: 31 words / frame 0x10 / GPR [31] / FPR none
  draft : 31 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_QuakeHit__18dEnTorideKokoopa_cFv
  target: 19 words / frame 0x10 / GPR [31] / FPR none
  draft : 19 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== finalizeState_QuakeHit__18dEnTorideKokoopa_cFv
  target: 4 words / frame none / GPR none / FPR none
  draft : 4 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_QuakeHit__18dEnTorideKokoopa_cFv
  target: 4 words / frame none / GPR none / FPR none
  draft : 4 words / frame none / GPR none / FPR none
  DIFFS 0
=== initializeState_SlideHit__18dEnTorideKokoopa_cFv
  target: 47 words / frame 0x20 / GPR [31] / FPR none
  draft : 47 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 5 naming artifact(s))
=== finalizeState_SlideHit__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_SlideHit__18dEnTorideKokoopa_cFv
  target: 31 words / frame 0x10 / GPR [31] / FPR none
  draft : 31 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_ShellHit__18dEnTorideKokoopa_cFv
  target: 47 words / frame 0x20 / GPR [31] / FPR none
  draft : 47 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 5 naming artifact(s))
=== finalizeState_ShellHit__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_ShellHit__18dEnTorideKokoopa_cFv
  target: 31 words / frame 0x10 / GPR [31] / FPR none
  draft : 31 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_ShellAtk_St__18dEnTorideKokoopa_cFv
  target: 127 words / frame 0x20 / GPR [30, 31] / FPR none
  draft : 127 words / frame 0x20 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 12 naming artifact(s))
=== shellinSE__18dEnTorideKokoopa_cFv
  target: 8 words / frame none / GPR none / FPR none
  draft : 8 words / frame none / GPR none / FPR none
  DIFFS 0
=== finalizeState_ShellAtk_St__18dEnTorideKokoopa_cFv
  target: 4 words / frame none / GPR none / FPR none
  draft : 4 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_ShellAtk_St__18dEnTorideKokoopa_cFv
  target: 153 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 153 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0  (+ 6 naming artifact(s))
=== shellatkSE__18dEnTorideKokoopa_cFv
  target: 8 words / frame none / GPR none / FPR none
  draft : 8 words / frame none / GPR none / FPR none
  DIFFS 0
=== shelllandonSE__18dEnTorideKokoopa_cFv
  target: 8 words / frame none / GPR none / FPR none
  draft : 8 words / frame none / GPR none / FPR none
  DIFFS 0
=== initializeState_ShellAtk__18dEnTorideKokoopa_cFv
  target: 69 words / frame 0x20 / GPR [31] / FPR [31]
  draft : 69 words / frame 0x20 / GPR [31] / FPR [31]
  DIFFS 0  (+ 3 naming artifact(s))
=== finalizeState_ShellAtk__18dEnTorideKokoopa_cFv
  target: 4 words / frame none / GPR none / FPR none
  draft : 4 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_ShellAtk__18dEnTorideKokoopa_cFv
  target: 117 words / frame 0x20 / GPR [31] / FPR [31]
  draft : 117 words / frame 0x20 / GPR [31] / FPR [31]
  DIFFS 0  (+ 3 naming artifact(s))
=== initializeState_ShellOut__18dEnTorideKokoopa_cFv
  target: 79 words / frame 0x20 / GPR [31] / FPR none
  draft : 79 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 5 naming artifact(s))
=== shelloutSE__18dEnTorideKokoopa_cFv
  target: 8 words / frame none / GPR none / FPR none
  draft : 8 words / frame none / GPR none / FPR none
  DIFFS 0
=== finalizeState_ShellOut__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_ShellOut__18dEnTorideKokoopa_cFv
  target: 100 words / frame 0x10 / GPR [31] / FPR none
  draft : 100 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== checkGetUp__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0
=== getupSE__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== getKokoopaOnFrm__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== getShellOffFrm__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_DieFumi_St__18dEnTorideKokoopa_cFv
  target: 64 words / frame 0x20 / GPR [31] / FPR none
  draft : 64 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== finalizeState_DieFumi_St__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DieFumi_St__18dEnTorideKokoopa_cFv
  target: 103 words / frame 0x30 / GPR [31] / FPR none
  draft : 103 words / frame 0x30 / GPR [31] / FPR none
  DIFFS 0  (+ 2 naming artifact(s))
=== initializeState_DieFire__18dEnTorideKokoopa_cFv
  target: 13 words / frame 0x10 / GPR [31] / FPR none
  draft : 13 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== finalizeState_DieFire__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DieFire__18dEnTorideKokoopa_cFv
  target: 19 words / frame 0x10 / GPR [31] / FPR none
  draft : 19 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== initializeState_DieShell__18dEnTorideKokoopa_cFv
  target: 13 words / frame 0x10 / GPR [31] / FPR none
  draft : 13 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== finalizeState_DieShell__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DieShell__18dEnTorideKokoopa_cFv
  target: 19 words / frame 0x10 / GPR [31] / FPR none
  draft : 19 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== hitShellDamageEffect__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== initializeState_DemoWait__18dEnTorideKokoopa_cFv
  target: 53 words / frame 0x20 / GPR [31] / FPR none
  draft : 53 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 3 naming artifact(s))
=== finalizeState_DemoWait__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DemoWait__18dEnTorideKokoopa_cFv
  target: 25 words / frame 0x10 / GPR [31] / FPR none
  draft : 25 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_DemoAwake__18dEnTorideKokoopa_cFv
  target: 54 words / frame 0x20 / GPR [31] / FPR none
  draft : 54 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 4 naming artifact(s))
=== finalizeState_DemoAwake__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DemoAwake__18dEnTorideKokoopa_cFv
  target: 51 words / frame 0x10 / GPR [31] / FPR none
  draft : 51 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== awakeSE__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== initializeState_DemoAwake_Wait__18dEnTorideKokoopa_cFv
  target: 49 words / frame 0x20 / GPR [31] / FPR none
  draft : 49 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 4 naming artifact(s))
=== finalizeState_DemoAwake_Wait__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DemoAwake_Wait__18dEnTorideKokoopa_cFv
  target: 30 words / frame 0x10 / GPR [31] / FPR none
  draft : 30 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_DemoIkaku__18dEnTorideKokoopa_cFv
  target: 54 words / frame 0x20 / GPR [31] / FPR none
  draft : 54 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 4 naming artifact(s))
=== finalizeState_DemoIkaku__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DemoIkaku__18dEnTorideKokoopa_cFv
  target: 56 words / frame 0x10 / GPR [31] / FPR none
  draft : 56 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== ikakuSE__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== ikakuEffect__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== initializeState_DemoIkaku_Wait__18dEnTorideKokoopa_cFv
  target: 49 words / frame 0x20 / GPR [31] / FPR none
  draft : 49 words / frame 0x20 / GPR [31] / FPR none
  DIFFS 0  (+ 4 naming artifact(s))
=== finalizeState_DemoIkaku_Wait__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DemoIkaku_Wait__18dEnTorideKokoopa_cFv
  target: 30 words / frame 0x10 / GPR [31] / FPR none
  draft : 30 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== initializeState_DemoEscape_St__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== finalizeState_DemoEscape_St__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== executeState_DemoEscape_St__18dEnTorideKokoopa_cFv
  target: 1 words / frame none / GPR none / FPR none
  draft : 1 words / frame none / GPR none / FPR none
  DIFFS 0
=== setBattleReady__18dEnTorideKokoopa_cFv
  target: 21 words / frame 0x10 / GPR [31] / FPR none
  draft : 21 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== tenmetsuFin__18dEnTorideKokoopa_cFv
  target: 9 words / frame none / GPR none / FPR none
  draft : 9 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== tenmetsuProc__18dEnTorideKokoopa_cFv
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== getShellOnFrm__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== getKokoopaOffFrm__18dEnTorideKokoopa_cCFv
  target: 2 words / frame none / GPR none / FPR none
  draft : 2 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== isStarInvalid__18dEnTorideKokoopa_cCFv
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== isFireInvalid__18dEnTorideKokoopa_cCFv
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== isFumiInvalid__18dEnTorideKokoopa_cCFv
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== getLookatPos__18dEnTorideKokoopa_cCFv
  target: 5 words / frame none / GPR none / FPR none
  draft : 5 words / frame none / GPR none / FPR none
  DIFFS 0
=== "__sinit_\d_enemy_toride_kokoopa_cpp"
  target: 1446 words / frame 0x420 / GPR none / FPR none / _savegpr_25
  draft : 1446 words / frame 0x420 / GPR none / FPR none / _savegpr_25
  DIFFS 0  (+ 4 naming artifact(s))
=== "__dt__33sFStateID_c<18dEnTorideKokoopa_c>Fv"
  target: 22 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 22 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0
=== "__dt__40sFStateVirtualID_c<18dEnTorideKokoopa_c>Fv"
  target: 23 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 23 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0
=== "baseID_DieShell<9dEnBoss_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "baseID_DieFire<9dEnBoss_c>__Fv_RC12sStateIDIf_c"
  target: 3 words / frame none / GPR none / FPR none
  draft : 3 words / frame none / GPR none / FPR none
  DIFFS 0
=== "number__40sFStateVirtualID_c<18dEnTorideKokoopa_c>CFv"
  target: 55 words / frame 0x10 / GPR [31] / FPR none
  draft : 55 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== "superID__40sFStateVirtualID_c<18dEnTorideKokoopa_c>CFv"
  target: 56 words / frame 0x10 / GPR [31] / FPR none
  draft : 56 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== "isSameName__33sFStateID_c<18dEnTorideKokoopa_c>CFPCc"
  target: 34 words / frame 0x10 / GPR [30, 31] / FPR none
  draft : 34 words / frame 0x10 / GPR [30, 31] / FPR none
  DIFFS 0
=== "initializeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c"
  target: 12 words / frame 0x10 / GPR none / FPR none
  draft : 12 words / frame 0x10 / GPR none / FPR none
  DIFFS 0
=== "executeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c"
  target: 12 words / frame 0x10 / GPR none / FPR none
  draft : 12 words / frame 0x10 / GPR none / FPR none
  DIFFS 0
=== "finalizeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c"
  target: 12 words / frame 0x10 / GPR none / FPR none
  draft : 12 words / frame 0x10 / GPR none / FPR none
  DIFFS 0
=== getFumiRev__12FumiCcInfo_cFv
  target: 20 words / frame none / GPR none / FPR none
  draft : 20 words / frame none / GPR none / FPR none
  DIFFS 0
=== operate__21MugenComboFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c
  target: 15 words / frame none / GPR none / FPR none
  draft : 15 words / frame none / GPR none / FPR none
  DIFFS 0  (+ 1 naming artifact(s))
=== operate__20KokoopaSpFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c
  target: 86 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  draft : 86 words / frame 0x20 / GPR [29, 30, 31] / FPR none
  DIFFS 0  (+ 4 naming artifact(s))
=== __dt__20KokoopaSpFumiCheck_cFv
  target: 16 words / frame 0x10 / GPR [31] / FPR none
  draft : 16 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
=== __dt__21MugenComboFumiCheck_cFv
  target: 16 words / frame 0x10 / GPR [31] / FPR none
  draft : 16 words / frame 0x10 / GPR [31] / FPR none
  DIFFS 0
```

---

## 8. NOT REACHED

**None**. All three numbered items in the Round 32 work order were fully executed, measured, verified, and reported in full detail.
