# Round 31 Report: `d_enemy_toride_kokoopa` Jump Twins Allocation Mapping, `setQuakeDead` Redundant-Zero Analysis, & Landing Readiness Audit

## 1. Summary & Headline Metrics

- **Current Unit Standing (Round 31)**:
  - Total Functions in TU: **251** (31,876 bytes)
  - Matched Functions: **248 / 251 (98.80%)**
  - Matched Bytes: **30,816 / 31,876 bytes (96.67%)**
  - Disagreements vs 248 / 251: **None** (independently audited by `fndiff.py --all` and `poolcheck.py`).
  - Unmatched Functions: **3 / 251 (1,060 bytes total)**
    - `setQuakeDead__18dEnTorideKokoopa_cFv` (340 B / 85 words target)
    - `initializeState_Jump__18dEnTorideKokoopa_cFv` (360 B / 90 words target)
    - `initializeState_BigJump__18dEnTorideKokoopa_cFv` (360 B / 90 words target)
- **GAINED Functions**: **0** (No unmatched function reached 0 diffs this round).
- **LOST Functions**: **0** (Zero regressions across all 248 matched functions).
- **Constant Pool Verification (`poolcheck.py`)**:
  - `177 pooled constants compared by VALUE across 250 paired functions`
  - `0 mismatched, 0 could not be resolved on one side` (Exit code: 0 clean).

---

## 2. GAINED & LOST Sections

### GAINED Functions (0 Gained)
No functions reached full byte-identical closure in Round 31.

### LOST Functions (0 Lost)
Zero functions regressed or fell out of the matched set in Round 31.

### Explicit Artifact-Matched Pair Re-Check:
1. **`__sinit_\d_enemy_toride_kokoopa_cpp` (5,784 B / 1,446 insns / frame `0x420`):**
   - Raw Byte Diffs: **0** (1,446 / 1,446 instructions byte-identical)
   - Canonical Diffs in `fndiff.py`: **0** (+ 4 naming artifacts)
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

## 3. Work Order Item 1: `initializeState_Jump` and `initializeState_BigJump` Analysis

### 3.1. Target Allocation vs Draft Baseline
- **Symbol Profile**: 90 words / frame `0x30` / GPR saves `[30, 31]` / FPR saves `none`.
- **Symbol Map Evidence on `l_EnMuki`**:
  - `l_EnMuki = .sdata2:0x8042C480; // type:object size:0x2 data:byte` (`include/game/bases/d_enemy.hpp` declares `extern const s8 l_EnMuki[];`).
  - Retail accesses `l_EnMuki[mDirection]` via `lbzx r4, r3, r4` + `extsb r4, r4` and converts to float using magic double `0x4330000080000000` via stack pair `0x18(r1)` / `0x1c(r1)`.
- **Register Allocation Target vs Draft**:
  - **Retail Target**: `f0 = speed.y`, `f1 = rate`, `f2 = speed.x`, `f3 = stack int->float double`, `f4 = magic constant double`.
  - **Draft Baseline**: `f0 = speed.y`, `f1 = rate`, `f2 = stack int->float double`, `f3 = magic constant double`, `f4 = speed.x`.

### 3.2. Jump Twins Variant Exploration Table (≥5 New Variants)

| Variant # | Source Code Shape | `initializeState_Jump` Diffs | `initializeState_BigJump` Diffs | Observed FPR Allocation (`f2`, `f3`, `f4`) |
| :--- | :--- | :---: | :---: | :--- |
| **Starting 5-Diff Form** | `float rate = calcJumpRate(); float muki = (float)l_EnMuki[mDirection]; mSpeed.y = speed.y; float sx = speed.x; mSpeed.x = (muki * rate) * sx;` | **DIFFS 5** (+ 2 naming artifacts) | **DIFFS 5** (+ 2 naming artifacts) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`f0` = `speed.y`, `f1` = `rate`) |
| **Variant 1 (Component-wise in branches)** | Assign `speed.x`/`speed.y` component-wise in `if (flag)` branches rather than struct copy: `speed.x = mpParamJump->mJumpSpeed1.x; speed.y = mpParamJump->mJumpSpeed1.y;` with starting math | **DIFFS 5** (+ 2 naming artifacts) | **DIFFS 5** (+ 2 naming artifacts) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`f0` = `speed.y`, `f1` = `rate`) |
| **Variant 2 (Ternary struct initialization)** | `mVec2_c speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;` with starting math | **DIFFS 5** (+ 2 naming artifacts) | **DIFFS 5** (+ 2 naming artifacts) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`f0` = `speed.y`, `f1` = `rate`) |
| **Variant 3 (Reference ternary binding)** | `const mVec2_c &speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;` with starting math | **DIFFS 44** (+ 2 naming artifacts, -6 words length) | **DIFFS 44** (+ 2 naming artifacts, -6 words length) | `f2` = magic double, `f3` = `sx` (from `0x0(r31)`), `f0` = stack double reused (stack frame shortened to `0x20`) |
| **Variant 4 (Comma operator sequencing)** | `float sx = (mSpeed.y = speed.y, speed.x); mSpeed.x = (muki * rate) * sx;` | **DIFFS 5** (+ 2 naming artifacts) | **DIFFS 5** (+ 2 naming artifacts) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`f0` = `speed.y`, `f1` = `rate`) |
| **Variant 5 (Double-precision cast)** | `double muki = (double)l_EnMuki[mDirection]; mSpeed.y = speed.y; float sx = speed.x; mSpeed.x = (muki * rate) * sx;` | **DIFFS 22** (+ 2 naming artifacts, +1 word length) | **DIFFS 22** (+ 2 naming artifacts, +1 word length) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`fsub`/`fmul` in double with terminal `frsp f0, f0`) |
| **Variant 6 (Parenthesized product with inlined cast)** | `mSpeed.y = speed.y; mSpeed.x = (speed.x * rate) * (float)l_EnMuki[mDirection];` | **DIFFS 11** (+ 2 naming artifacts) | **DIFFS 11** (+ 2 naming artifacts) | `f0` = `speed.x`, `f1` = `rate` (then reused for int->float), `f2` = magic double, `f3` = `speed.y` |

### 3.3. Diagnostic Findings on Register Inversion
1. **The Allocation Inversion Root Cause**: When `(float)l_EnMuki[mDirection]` is evaluated, MWCC's code generator reserves two registers (`f3`, `f2` in the draft) for the subtraction `fsubs f0, f2, f3`. Because `float sx = speed.x` appears textually after `muki` in the C++ AST, MWCC allocates `f4` to `sx` when loading `0x10(r1)`.
2. **Impact of Hoisting `sx`**: Moving `float sx = speed.x` earlier before `muki` causes `sx` to land in `f4` or `f31` (if live across `calcJumpRate`), but does not reorder the int-to-float pair ahead of `speed.x` in retail's exact `f2`-first sequence without perturbing other instructions.
3. **Twin Consistency**: In all tested variants, `initializeState_Jump` and `initializeState_BigJump` behave identically across all register assignments, instruction counts, and diff structures.

---

## 4. Work Order Item 2: `setQuakeDead` Non-CSE Null & Redundant `li r3, 0` Analysis

### 4.1. The Narrow Technical Question
The `(fBase_c *)mUnk770` variant achieved the exact target frame (`0x30`), exact non-volatile GPR set (`[30, 31]`), and exact instruction sequence minus one word (84 words vs 85 target words). The missing word is `li r3, 0` at target instruction 36.
Retail generates:
```asm
800A9B14: lwz   r3, 0x770(r30)
800A9B18: cmpwi r3, 0x0
800A9B1C: bne   .L_call
800A9B20: li    r3, 0x0            ; <-- Redundant null materialised in r3
800A9B24: b     .L_check           ; <-- Branch into shared merge check
.L_call:
800A9B28: bl    searchBaseByID__10fManager_cF9fBaseID_e
.L_check:
800A9B2C: cmpwi r3, 0x0            ; <-- Single shared test on merged value
800A9B30: beq   .L_skip
800A9B34: bl    deleteRequest__7fBase_cFv
```

### 4.2. Measured Variants Comparison Table (≥3 Variants)

| Variant | Source Code Form | Words | Frame | Non-Volatile Set (GPR) | `li r3,0` + `b` -> shared `cmpwi` Present | Notes / Mechanism |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- |
| **Variant A (Different Pointer Type Null)** | `fBase_c *base = (mUnk770 == 0) ? (dActor_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != 0) base->deleteRequest();` | **88** | `0x40` | `[29, 30, 31]` | **No** (Hoists `r29`) | Literal 0 in pointer cast participates in global constant CSE across `UnKnownScoreSet`, saving constant 0 into `r29` and adding 0x10 to frame. |
| **Variant B (Condition on Local `id`, Argument `mUnk770`)** | `u32 id = mUnk770; fBase_c *base = (id == 0) ? (fBase_c*)mUnk770 : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != 0) base->deleteRequest();` | **84** | `0x30` | `[30, 31]` | **No** (Elides `li r3,0`) | Frame `0x30` and GPR `[30, 31]` preserved. MWCC value range analysis recognizes `r3` already holds 0 on the zero path, eliding `li r3,0` and jumping straight to `.L_check`. |
| **Variant C (Condition on `mUnk770`, Passed Local `id`)** | `fBaseID_e id = (fBaseID_e)mUnk770; fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID(id); if (base != 0) base->deleteRequest();` | **88** | `0x40` | `[29, 30, 31]` | **No** (Hoists `r29`) | Passing distinct name `id` does not hide literal 0 in false arm from global CSE, resulting in `r29` hoisting. |
| **Variant D (Null Arm via Right Shift `(mUnk770 >> 31)`)** | `fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 >> 31) : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != 0) base->deleteRequest();` | **85** | `0x30` | `[30, 31]` | **No** (Emits `srwi r3, r3, 31` + `b`) | **Exact 85 words, frame `0x30`, GPR `[30,31]`**. Generates `srwi r3, r3, 31` + `b .L_check` into shared `cmpwi r3, 0`. Exactly **1 instruction diff** vs target (`srwi` vs `li`). |
| **Variant E (Null Arm via Zero Member `(u32)mUnk790`)** | `fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(u32)mUnk790 : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != 0) base->deleteRequest();` | **85** | `0x30` | `[30, 31]` | **No** (Emits `lhz r3, 0x790(r30)` + `b`) | **Exact 85 words, frame `0x30`, GPR `[30,31]`**. Generates `lhz r3, 0x790(r30)` + `b .L_check` into shared `cmpwi r3, 0`. Exactly **1 instruction diff** vs target (`lhz` vs `li`). |

### 4.3. Findings on MWCC Redundant Constant Materialization
1. **The Optimization Paradox**: In C++, writing literal `0` / `nullptr` triggers MWCC global constant propagation which discovers 3 uses of constant 0 spanning across `UnKnownScoreSet` and allocates `r29`. Writing a variable expression like `(fBase_c*)mUnk770` avoids constant propagation (retaining frame `0x30` and GPR `[30, 31]`), but MWCC's copy-propagation / value-tracking detects that `r3` already contains 0 from `lwz r3, 0x770(r30)` and removes the redundant `li r3, 0`.
2. **The 85-Word Frame `0x30` Bridge**: Variants D and E prove that any non-constant zero expression that computes 0 without constant literal CSE yields the **exact 85 words / frame `0x30` / GPR `[30, 31]` profile**, isolating the discrepancy down to a single instruction in the ternary merge path.

---

## 5. Work Order Item 3: Landing Readiness Statement

### Plain Statement: Is this unit landable with the current object?
### **NO.**

### Detailed Landing Gate Analysis:
Under `tools/auto_decomp/land.py`, the landing gate requires that the unit form a single **contiguous address range per section** and satisfy:
```
ninja && python progress.py --verify-bin -> 5/5 binaries hash-identical
```
There is no non-matching hole-punching mechanism in `land.py`. **248 / 251 matched functions cannot land. 251 / 251 matched functions are required.**

### Exact Remaining Unmatched List (3 Functions / 1,060 Bytes Total):
1. **`initializeState_Jump__18dEnTorideKokoopa_cFv`**
   - Section Address: `0x800ABA40` (Size: 360 bytes / 90 words)
   - Current Status: **5 instruction diffs** (register allocation inversion `f2` vs `f4`)
2. **`initializeState_BigJump__18dEnTorideKokoopa_cFv`**
   - Section Address: `0x800ABE00` (Size: 360 bytes / 90 words)
   - Current Status: **5 instruction diffs** (exact twin of `initializeState_Jump`)
3. **`setQuakeDead__18dEnTorideKokoopa_cFv`**
   - Section Address: `0x800A9A90` (Size: 340 bytes / 85 words)
   - Current Status: **80 diffs** (with literal ternary hoisting `r29`) / **1 diff** (with non-CSE zero expression)

---

## 6. Constant Pool Verification Output (`poolcheck.py`)

```
177 pooled constants compared by VALUE across 250 paired functions
0 mismatched, 0 could not be resolved on one side
(248 pair(s) value-checked; 16 reference(s) skipped as the same named symbol on both sides; 381 float load(s) seen; 1 pair(s) skipped on length)
COVERAGE: 248 of 488 target function(s) value-checked; 240 were not checked at all (unpaired, length-mismatched, or already differing).
```

---

## 7. Verbatim `fndiff.py --all` Output

```
﻿=== "baseID_Jump_St<10sStateID_c>__Fv_RC12sStateIDIf_c"
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
  draft : 88 words / frame 0x40 / GPR [29, 30, 31] / FPR none
  DIFFS 80  (+ +3 words of length difference)
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

## 8. Work Order Completeness & Not Reached List

- **Items Attempted**:
  - Item 1 (`initializeState_Jump` and `initializeState_BigJump` sweep & FPR allocation mapping): **Completed in full** (6 new variants evaluated and mapped).
  - Item 2 (`setQuakeDead` redundant-zero & non-CSE analysis): **Completed in full** (5 new variants evaluated across words, frame, GPRs, and merge pattern).
  - Item 3 (Landing readiness statement and remaining blocker audit): **Completed in full**.
- **`NOT REACHED`**: **None** (all three numbered work order items were reached, measured, and reported in full).
