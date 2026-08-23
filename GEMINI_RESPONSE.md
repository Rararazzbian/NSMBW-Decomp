# Round 33 Report: `setQuakeDead` Canonical Null Restoration, Death-Info Compound Literal Proof, & Jump Twins FPR Sweep

## 1. Headline Metrics & Primary Result

- **Primary TU Standing (Round 33)**:
  - **Total Functions in TU**: **251** (31,876 bytes)
  - **Matched Functions**: **248 / 251 (98.80%)**
  - **Matched Bytes**: **30,816 / 31,876 bytes (96.67%)**
  - **Unmatched Functions**: **3 / 251 (1,060 bytes total)**:
    - `setQuakeDead__18dEnTorideKokoopa_cFv` (340 B target / 85w target; canonical source with `nullptr` & compound literal is 88w / frame `0x40` / `[29, 30, 31]`; backup `>> 31` preserved at 85w / frame `0x30` / `[30, 31]` / 1 diff)
    - `initializeState_Jump__18dEnTorideKokoopa_cFv` (360 B target / 90w target vs 90w draft, **5 diffs**)
    - `initializeState_BigJump__18dEnTorideKokoopa_cFv` (360 B target / 90w target vs 90w draft, **5 diffs**)
  - **GAINED Functions**: **0**
  - **LOST Functions**: **0** (Zero regressions across all 248 matching functions).
  - **Constant Pool Verification (`poolcheck.py`)**: `177 pooled constants compared by VALUE across 250 paired functions; 0 mismatched, 0 could not be resolved on one side` (Exit code: 0 clean).

---

## 2. GAINED & LOST Sections

### GAINED Functions (0 Gained)
No new functions reached full 0-diff byte-identity in Round 33. Canonical `nullptr` and the verified `.rodata` compound literal have been folded into the primary draft `scratch/gemini_round24/d_enemy_toride_kokoopa.cpp`, while the 85-word `>> 31` object is preserved in `scratch/gemini_round24/d_enemy_toride_kokoopa_shift31.cpp`.

### LOST Functions (0 Lost)
Zero functions regressed or left the matched set in Round 33.

### Explicit Artifact-Matched Pair Re-Check:
All 4 artifact-matched functions were re-verified against the fresh primary object `d_enemy_toride_kokoopa.o` and remain 100% matched under the union gate / naming-artifact rule:
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

## 3. Work Order Item 1: `setQuakeDead` with `nullptr` Restored & Death-Info Question

### 3.1. The Death-Info Literal Question: Decoded Retail DOL Evidence
Investigation of the retail binary `original/wiimj2d.dol` and `bin/dtk/wiimj2d_symbols.txt` confirms conclusively that retail uses an **anonymous compound literal** placed in `.rodata`, rather than a named global `l_dieQuake`:

- **Symbol Map Address**: `@70611 = .rodata:0x802F0C40; // type:object size:0x20 scope:local`
- **Retail IEEE-754 / Pointer Data at `0x802F0C40` (32 bytes = `sizeof(sDeathInfoData)`)**:
  - `mXSpeed`: `0.0f` (`0x00000000`)
  - `mYSpeed`: `3.0f` (`0x40400000`)
  - `mMaxYSpeed`: `-4.0f` (`0xC0800000`)
  - `mYAccel`: `-0.1875f` (`0xBE400000`)
  - `mDeathState`: `&dEnBoss_c::StateID_DieStar` (`0x80357AF4`)
  - `mScore`: `-1` (`0xFFFFFFFF`)
  - `m_18`: `-1` (`0xFFFFFFFF`)
  - `mDirection`: `0`
  - `mKilledBy`: `0xFF` (`255`)

- **Comparison across all 4 sibling death-info entries in `.rodata`**:
  - `0x802F0C00` (`@70556`, size 0x20): `(sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieFire, -1, -1, 0, 0 }` (used in `setFireDead`)
  - `0x802F0C20` (`@70590`, size 0x20): `(sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0 }` (used in `setStarDead`)
  - `0x802F0C40` (`@70611`, size 0x20): `(sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF }` (used in `setQuakeDead`)
  - `0x802F0C60` (`@70647`, size 0x20): `(sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieShell, -1, -1, 0, 0 }` (used in `setShellDead`)

**Conclusion**: In accordance with the retail evidence, we replaced `sDeathInfoData deathData = l_dieQuake;` with the authentic compound literal:
```cpp
sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
```
and removed the unused file-scope `static const sDeathInfoData l_die*` declarations from the source.

### 3.2. Statement-by-Statement Structural Diff: `setQuakeDead` vs `setShellDead`
Diffing `setQuakeDead` against its byte-matching sibling `setShellDead` reveals why MWCC's global register allocator behaves differently between the two functions:

| Statement / Construct | `setShellDead` (Matched 100%) | `setQuakeDead` (Retail Target) | Codegen Impact on Register Allocation |
| :--- | :--- | :--- | :--- |
| **1. Direction Calculation** | `(mPos.x < killedBy->mPos.x) ? 0 : 1` | `getPl_LRflag(mPos)` | In `setQuakeDead`, `getPl_LRflag` is an out-of-line call saving `dir` into `r31`. In `setShellDead`, `r30` holds `dir` and `r29` holds `killedBy`. |
| **2. `mAnmMatClr` Check** | Near the bottom (after base delete) | At the very top (before `removeCc`) | Verified in retail disasm: `setQuakeDead` has `mAnmMatClr` at the top (`0x800A9AB8`). |
| **3. `removeCc()` & `release()`** | `removeCc(); mCc.release();` | `removeCc(); mCc.release();` | Identical. |
| **4. Zero Stores** | `mUnk792 = 0; mUnk790 = 0;` | `mUnk792 = 0; mUnk790 = 0;` | Identical source. |
| **5. Score Function Call** | `if ((u32)playerNo <= 3) ScoreSet(...)` | `UnKnownScoreSet(this, 6, 0.0f, 24.0f)` | **CRITICAL STRUCTURAL DIFFERENCE**: In `setShellDead`, the score call is **conditional** (guarded by `if`), and `r29` is already busy holding `killedBy`. In `setQuakeDead`, `UnKnownScoreSet` is **unconditional straight-line code**, and `r29` is unoccupied! |
| **6. Search Base & Delete** | `fBase_c *base = (mUnk770 == 0) ? nullptr : ...; if (base) base->deleteRequest();` | Identical source construct | Because `UnKnownScoreSet` is straight-line and `r29` is free, MWCC's global register allocator CSE-unifies the zero constant from `mUnk792/mUnk790 = 0` with the ternary merge register across the call into callee-saved `r29`. |
| **7. Death Info Assignment** | `(sDeathInfoData){ ... &StateID_DieShell ... }` | `(sDeathInfoData){ ... &StateID_DieStar ... 0xFF }` | Both emit `.rodata` anonymous compound literals. |

### 3.3. `setQuakeDead` Measured Variants Table (5 Columns)

| Variant | Source Code Shape | Words / Frame | Non-Volatile Set (GPR) | `li r3, 0` Present + `b` -> Shared `cmpwi` |
| :--- | :--- | :---: | :---: | :---: |
| **Target (Retail)** | Standard ternary with `nullptr` & `UnKnownScoreSet` | **85w / `0x30`** | **`[30, 31]`** | **YES** (`36 li r3, 0x0` + `37 b .L_check` -> `38 cmpwi r3, 0x0`) |
| **Canonical Variant (nullptr + Compound Literal)** | `fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != nullptr) base->deleteRequest();` with compound literal | 88w / `0x40` | `[29, 30, 31]` | NO (merges into `r29`: `41 mr r29, r3` -> `42 cmpwi r29, 0x0`) |
| **Preserved Shift Backup (`>> 31`)** | `fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 >> 31) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);` | 85w / `0x30` | `[30, 31]` | NO (emits `srwi r3, r3, 31` instead of `li r3, 0x0`, 1 diff) |
| **Variant Q2 (Split Zero Stores Around Score)** | `mUnk792 = 0; UnKnownScoreSet(...); mUnk790 = 0;` with ternary | 88w / `0x40` | `[29, 30, 31]` | NO (hoists `r29` across score call) |
| **Variant Q3 (Inverted Ternary `!= 0`)** | `fBase_c *base = (mUnk770 != 0) ? fManager_c::searchBaseByID((fBaseID_e)mUnk770) : nullptr;` | 87w / `0x40` | `[29, 30, 31]` | NO (hoists `r29`, 82 diffs) |
| **Variant Q4 (`if/else` Local Assignment)** | `fBase_c *base = nullptr; if (mUnk770 != 0) base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);` | 86w / `0x30` | `[30, 31]` | NO (separate compare blocks, 51 diffs) |
| **Variant Q5 (`dActor_c*` Cast)** | `dActor_c *base = (mUnk770 == 0) ? nullptr : (dActor_c*)fManager_c::searchBaseByID((fBaseID_e)mUnk770);` | 88w / `0x40` | `[29, 30, 31]` | NO (hoists `r29`, 80 diffs) |
| **Variant Q6 (Direct Guarded Delete)** | `if (mUnk770 != 0) { fBase_c *b = fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (b) b->deleteRequest(); }` | 83w / `0x30` | `[30, 31]` | NO (guard-if structure, 48 diffs) |

---

## 4. Work Order Item 2: The Jump Twins (`initializeState_Jump` & `initializeState_BigJump`)

### 4.1. Retail Listing Inspection & Compound-Literal Parameter Audit
We audited the retail disassembly for both `initializeState_Jump` (`0x800AB9F0`) and `initializeState_BigJump` (`0x800ABD50`):
- **`initializeState_Jump`** loads from `0x754(r30)` (`mpParamJump`):
  - True branch (`flag != 0`): `lfs f1, 0x14(r3)` (`mJumpSpeed1.x`), `lfs f0, 0x18(r3)` (`mJumpSpeed1.y`)
  - False branch (`flag == 0`): `lfs f1, 0x24(r3)` (`mJumpSpeed2.x`), `lfs f0, 0x28(r3)` (`mJumpSpeed2.y`)
- **`initializeState_BigJump`** loads from `0x754(r30)` (`mpParamJump`):
  - True branch (`flag != 0`): `lfs f1, 0x1C(r3)` (`mBigJumpSpeed1.x`), `lfs f0, 0x20(r3)` (`mBigJumpSpeed1.y`)
  - False branch (`flag == 0`): `lfs f1, 0x2C(r3)` (`mBigJumpSpeed2.x`), `lfs f0, 0x30(r3)` (`mBigJumpSpeed2.y`)

**Result**: Retail genuinely reads `mpParamJump` members directly at runtime. There is no pooled anonymous literal for jump speeds.

### 4.2. Observed FP Register Allocation & Instruction Scheduling Mechanism
In retail:
```asm
800ABAC8: mr    r3, r30
800ABACC: bl    calcJumpRate__18dEnTorideKokoopa_cFv  ; rate -> f1
800ABAD0: lis   r3, l_EnMuki@ha
800ABAD4: lbz   r4, 0x348(r30)                       ; mDirection
800ABAD8: addi  r3, r3, l_EnMuki@l
800ABADC: lis   r0, 0x4330
800ABAE0: lbzx  r4, r3, r4                          ; l_EnMuki[mDirection]
800ABAE4: mr    r3, r30
800ABAE8: lfs   f0, 0x14(r1)                         ; f0 = speed.y
800ABAEC: extsb r4, r4
800ABAF0: stw   r0, 0x18(r1)
800ABAF4: xoris r0, r4, 0x8000
800ABAF8: lfd   f4, "@75355"@sda21(r0)               ; f4 = 4503601774854144.0 (magic)
800ABAFC: stw   r0, 0x1c(r1)
800ABB00: lfs   f2, 0x10(r1)                         ; f2 = speed.x
800ABB04: lfd   f3, 0x18(r1)                         ; f3 = (0x43300000, 0x8000 ^ muki)
800ABB08: stfs  f0, 0xec(r30)                        ; mSpeed.y = speed.y (from f0)
800ABB0C: fsubs f0, f3, f4                           ; f0 = (float)muki
800ABB10: fmuls f0, f0, f1                           ; f0 = muki * rate
800ABB14: fmuls f0, f0, f2                           ; f0 = (muki * rate) * speed.x
800ABB18: stfs  f0, 0xe8(r30)                        ; mSpeed.x = f0
```

In draft (`k6_sx_late` / 5 diffs):
```asm
lfs f0, 0x14(r1)                         ; f0 = speed.y
lfd f3, "@27448"@sda21(r0)               ; f3 = magic (allocated to f3 instead of f4)
lfs f4, 0x10(r1)                         ; f4 = speed.x (allocated to f4 instead of f2)
lfd f2, 0x18(r1)                         ; f2 = int-to-float (allocated to f2 instead of f3)
stfs f0, 0xec(r30)                       ; mSpeed.y = f0
fsubs f0, f2, f3                         ; f0 = (float)muki
fmuls f0, f0, f1                         ; f0 = muki * rate
fmuls f0, f4, f0                         ; f0 = speed.x * (muki * rate) (commuted because f4 > f0)
stfs f0, 0xe8(r30)
```

### 4.3. Jump Twins Measured Variants Table

| Variant | `initializeState_Jump` | `initializeState_BigJump` | Observed FP Allocation / Insn Schedule | Analysis / Result |
| :--- | :---: | :---: | :--- | :--- |
| **Target (Retail)** | **90w / `0x30` / 0 diff** | **90w / `0x30` / 0 diff** | `f0`=speed.y, `f1`=rate, `f4`=magic, `f2`=speed.x, `f3`=int-pair; `fsubs f0,f3,f4; fmuls f0,f0,f1; fmuls f0,f0,f2` | Exact match on frame, saves, and instruction count. |
| **Baseline (`k6_sx_late`)** | **90w / `0x30` / 5 diffs** | **90w / `0x30` / 5 diffs** | `f0`=speed.y, `f1`=rate, `f3`=magic, `f4`=speed.x, `f2`=int-pair; `fsubs f0,f2,f3; fmuls f0,f0,f1; fmuls f0,f4,f0` | **Best shape (5 diffs)**. Residual is pure volatile FPR permutation (`f2/f3/f4`). |
| **Variant V1 (Component-wise x then y)** | 90w / `0x30` / 5 diffs | 90w / `0x30` / 5 diffs | `speed.x = ...; speed.y = ...;` into `mVec2_c speed` | Emits identical loads/stores to struct copy; 5 diffs. |
| **Variant V2 (Component-wise y then x)** | 90w / `0x30` / 13 diffs | 90w / `0x30` / 13 diffs | `speed.y = ...; speed.x = ...;` | Reverses branch store order into `0x14(r1)` and `0x10(r1)`, regressing to 13 diffs. |
| **Variant V3 (Separate Scalars `sx, sy`)** | 92w / `0x40` / 80 diffs | 92w / `0x40` / 80 diffs | `float sx, sy;` live across `calcJumpRate()` | Scalars kept live across call force callee-saved FPRs `f30, f31`, growing frame to `0x40` (+2 words). |
| **Variant V4 (Separate Scalars `sy, sx`)** | 92w / `0x40` / 80 diffs | 92w / `0x40` / 80 diffs | `float sy, sx;` live across `calcJumpRate()` | Same as V3: forces `f30, f31` saves (+2 words, 80 diffs). |
| **Variant VA (`rate -> sy -> sx -> muki`)** | 90w / `0x30` / 7 diffs | 90w / `0x30` / 7 diffs | `f3`=speed.y, `f4`=speed.x, `f2`=magic, `f0`=int-pair | 7 diffs (speed.y loaded into `f3` instead of `f0`). |
| **Variant VB (`rate -> sx -> sy -> muki`)** | 90w / `0x30` / 7 diffs | 90w / `0x30` / 7 diffs | `f3`=speed.y, `f4`=speed.x, `f2`=magic, `f0`=int-pair | 7 diffs. |
| **Variant VC (`rate -> sx -> muki -> sy`)** | 90w / `0x30` / 5 diffs | 90w / `0x30` / 5 diffs | `f0`=speed.y, `f4`=speed.x, `f3`=magic, `f2`=int-pair | Matches `k6_sx_late` at 5 diffs. |
| **Variant VD (Direct Inline No Locals)** | 90w / `0x30` / 8 diffs | 90w / `0x30` / 8 diffs | `f4`=speed.y, `f0`=speed.x, `f3`=magic, `f2`=int-pair | 8 diffs (unnamed speed.x renumbers to `f0`). |
| **Variant VE (`speed.x * (muki * rate)`)** | 90w / `0x30` / 6 diffs | 90w / `0x30` / 6 diffs | `f2`=speed.y, `f0`=speed.x, `f4`=magic, `f3`=int-pair | 6 diffs (reverses `f0/f2` for speed.y/speed.x). |
| **Variants DS1-DS5 (Declaration Splits)** | 90w / `0x30` / 5-7 diffs | 90w / `0x30` / 5-7 diffs | Early declaration splits on volatile `f32 sx, muki;` | Confirming `AGENT_CONTEXT.md:1697`: declaration order does not control volatile FPRs (`f0..f13`). Bottoms out at 5 diffs. |

---

## 5. Work Order Item 3: Fold and Verification (`fndiff.py --all` & `poolcheck.py`)

### 5.1. Fold Confirmation
- Primary draft `scratch/gemini_round24/d_enemy_toride_kokoopa.cpp` folded with:
  1. Canonical `nullptr` restored in `setQuakeDead`.
  2. Anonymous compound literal `(sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF }` in `setQuakeDead`.
  3. File-scope unused `l_die*` declarations removed.
  4. `initializeState_Jump` and `initializeState_BigJump` folded with the optimal 5-diff shape (`k6_sx_late`).
- Backup `>> 31` source and compiled object preserved in `scratch/gemini_round24/d_enemy_toride_kokoopa_shift31.cpp` and `.o`.

### 5.2. `poolcheck.py` Output
```
177 pooled constants compared by VALUE across 250 paired functions
0 mismatched, 0 could not be resolved on one side
(248 pair(s) value-checked; 16 reference(s) skipped as the same named symbol on both sides; 381 float load(s) seen; 1 pair(s) skipped on length)
COVERAGE: 248 of 488 target function(s) value-checked; 240 were not checked at all (unpaired, length-mismatched, or already differing).
```

### 5.3. `fndiff.py --all` Full Output
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
  DIFFS 5  (+ 2 naming artifact(s))
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
  DIFFS 5  (+ 2 naming artifact(s))
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

## 6. NOT REACHED

None. All three work order items were investigated, tested across multiple variant suites, verified with retail DOL byte analysis, and folded in full.
