import os

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'gemini_round24')

with open(os.path.join(BASE, 'full_fndiff_all.txt'), 'r', encoding='utf-8') as f:
    fndiff_all = f.read()

with open(os.path.join(BASE, 'full_poolcheck.txt'), 'r', encoding='utf-8') as f:
    poolcheck = f.read().strip()

parts = []
parts.append("""# Round 34 Report: `setQuakeDead` Allocator Deconstruction & Register Sweep, Jump Twins Provenance Experiments, and Full TU Verification

## 1. Headline Metrics & Primary Result

- **Primary TU Standing (Round 34)**:
  - **Total Functions in TU**: **251** (31,876 bytes)
  - **Matched Functions**: **248 / 251 (98.80%)**
  - **Matched Bytes**: **30,816 / 31,876 bytes (96.67%)**
  - **Unmatched Functions**: **3 / 251 (1,060 bytes total)**:
    - `setQuakeDead__18dEnTorideKokoopa_cFv` (340 B target / 85w target; canonical source with `nullptr` & compound literal is 88w / frame `0x40` / GPR `[29, 30, 31]` / 80 diffs [positional] = 78/85 identical instructions via alignment-aware edit script; closest honest non-volatile shape is Claude's `q5_null_then_if` at 86w / frame `0x30` / GPR `[30, 31]` / merge in `r0`)
    - `initializeState_Jump__18dEnTorideKokoopa_cFv` (360 B target / 90w target vs 90w draft, **5 diffs** / 0 words length difference / frame `0x30` / GPR `[30, 31]`)
    - `initializeState_BigJump__18dEnTorideKokoopa_cFv` (360 B target / 90w target vs 90w draft, **5 diffs** / 0 words length difference / frame `0x30` / GPR `[30, 31]`)
  - **GAINED Functions**: **0**
  - **LOST Functions**: **0** (Zero regressions across all 248 matching functions).
  - **Constant Pool Verification (`poolcheck.py`)**: `177 pooled constants compared by VALUE across 250 paired functions; 0 mismatched, 0 could not be resolved on one side` (Exit code: 0 clean).

---

## 2. Work Order Item 1: `setQuakeDead` Allocator Analysis & Variant Sweep

### 2.1. Structural & Allocator Deconstruction
In Round 33/34, an alignment-aware edit script (`difflib.SequenceMatcher`) proved that **78 of 85 instructions in `setQuakeDead` are 100% identical** between the canonical draft and retail. Head matches, tail matches, and the 32-instruction compound literal death-info block at the end is byte-exact.

Every single difference traces to one register allocation decision:
1. In retail, `mUnk792 = 0; mUnk790 = 0;` uses volatile `r0` (`li r0, 0x0; sth r0, 0x792(r30); sth r0, 0x790(r30)`).
2. In retail, `fBase_c *base = (mUnk770 == 0) ? nullptr : searchBaseByID(...)` evaluates with `cmpwi r3, 0; bne .L_call; li r3, 0; b .L_merge; .L_call: bl searchBaseByID; .L_merge: cmpwi r3, 0; beq .L_skip; bl deleteRequest`. The null arm explicitly materializes `li r3, 0x0`.
3. In the draft, MWCC notices that callee-saved `r29` is unoccupied across `UnKnownScoreSet`. It places the zero constant for `mUnk792/mUnk790 = 0` into `r29` (`li r29, 0x0`), preserves `r29` across `UnKnownScoreSet`, and then reuses `r29` as the merge register for the ternary (`mr r29, r3`), completely omitting the `li r3, 0` on the false arm.

In `setShellDead`, by contrast, `killedBy` is live across `removeCc()`, `mCc.release()`, and the zero stores, occupying `r29`. Because `r29` is occupied by `killedBy`, MWCC refuses to allocate a fourth callee-saved register (`r28`) just for a constant zero, and naturally falls back to `r0` and `li r3, 0`. In `setQuakeDead`, `r29` is unconstrained, triggering MWCC's greedy global allocation heuristic.

### 2.2. Measured `setQuakeDead` Variants Table

| Variant | Source Code Shape / Description | Store-Constant Register | Merge Register | `li r3, 0` Present | Words | Frame | Non-Volatile GPR Set |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Target (Retail)** | Standard ternary with `nullptr` & `UnKnownScoreSet` | **`r0`** | **`r3`** | **YES** | **85w** | **`0x30`** | **`[30, 31]`** |
| **Canonical Baseline** | `fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != nullptr) base->deleteRequest();` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **V1_reg_base** | `register fBase_c *base = (mUnk770 == 0) ? nullptr : ...` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **V1b_reg_zero** | `register u32 z = 0; mUnk792 = z; mUnk790 = z;` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **V1c_reg_this** | `register dEnTorideKokoopa_c *th = this;` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **V2_scope_zero** | `{ u16 z = 0; mUnk792 = z; mUnk790 = z; }` (nested block) | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **V2b_scope_base** | `{ fBase_c *base = ...; if (base) base->deleteRequest(); }` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **V3_store_u32** | `*(u32*)&mUnk790 = 0;` (32-bit word store) | `r29` | `r29` | NO | 87w | `0x40` | `[29, 30, 31]` |
| **V4b_score_local** | `dScoreMng_c *sm = dScoreMng_c::m_instance; sm->UnKnownScoreSet(...);` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **V5_claude_q5** | `fBase_c *base = nullptr; if (mUnk770 != 0) base = searchBaseByID(...); if (base) base->deleteRequest();` | **`r0`** | **`r0`** | NO | 86w | **`0x30`** | **`[30, 31]`** |
| **V5b_if_else_init** | `fBase_c *base; if (mUnk770 == 0) base = nullptr; else base = searchBaseByID(...);` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **Dir_late** | `getPl_LRflag(mPos)` placed late after base deletion | `r31` | `r31` | NO | 86w | **`0x30`** | **`[30, 31]`** |
| **Ternary_id_local** | `fBaseID_e id = (fBaseID_e)mUnk770; fBase_c *base = (id == 0) ? nullptr : searchBaseByID(id);` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **Ternary_u32_cond** | `fBase_c *base = ((u32)mUnk770 == 0) ? nullptr : ...` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **Ternary_not_cond** | `fBase_c *base = (!mUnk770) ? nullptr : ...` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **Ternary_zero_literal**| `fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : ...` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **Ternary_in_if** | `if (fBase_c *base = (mUnk770 == 0) ? nullptr : ...) base->deleteRequest();` | `r29` | `r29` | NO | 88w | `0x40` | `[29, 30, 31]` |
| **Ternary_reorder_score**| Base deletion placed *before* `UnKnownScoreSet` (probe) | **`r0`** | **`r3`** | **YES** | **85w** | **`0x30`** | **`[30, 31]`** |

---

## 3. Work Order Item 2: The Twins (`initializeState_Jump` and `initializeState_BigJump`)

### 3.1. Register Inversion & Provenance Analysis
Both twins (`initializeState_Jump` and `initializeState_BigJump`) are 90 words long, have matching frame size `0x30`, save identical non-volatile registers `[30, 31]`, and differ by exactly 5 instructions.

The residual is an instruction scheduling / register assignment inversion between `speed.x` and the integer-to-float magic conversion of `l_EnMuki[mDirection]`:
- **Retail Target**:
  - `speed.y` loaded into `f0` (`lfs f0, 0x14(r1)`)
  - `calcJumpRate()` returns in `f1`
  - Magic constant loaded into `f4` (`lfd f4, @75355@sda21(r0)`)
  - `speed.x` loaded into `f2` (`lfs f2, 0x10(r1)`)
  - Converted integer loaded into `f3` (`lfd f3, 0x18(r1)`)
  - `stfs f0, 0xec(r30)` (`mSpeed.y = speed.y`)
  - `fsubs f0, f3, f4` -> `f0 = muki`
  - `fmuls f0, f0, f1` -> `f0 = muki * rate`
  - `fmuls f0, f0, f2` -> `f0 = (muki * rate) * speed.x` (operands in order because `f0 < f2`)
  - `stfs f0, 0xe8(r30)` (`mSpeed.x = ...`)
- **Draft**:
  - `speed.y` loaded into `f0` (`lfs f0, 0x14(r1)`)
  - `calcJumpRate()` returns in `f1`
  - Converted integer loaded into `f2` (`lfd f2, 0x18(r1)`)
  - Magic constant loaded into `f3` (`lfd f3, @sda21(r0)`)
  - `fsubs f0, f2, f3` -> `f0 = muki`
  - `speed.x` loaded into `f4` (`lfs f4, 0x10(r1)`)
  - `fmuls f0, f0, f1` -> `f0 = muki * rate`
  - `fmuls f0, f4, f0` -> `f0 = speed.x * (muki * rate)` (operands commuted because `f4 > f0`)
  - `stfs f0, 0xe8(r30)` (`mSpeed.x = ...`)

### 3.2. Measured Twins Variants Table

| Variant | Source Code Shape / Description | Jump Diffs (Score) | BigJump Diffs (Score) | Observed FPR Allocation (`magic`, `speed.x`, `int->float`, `fsubs`, `fmuls`) |
| :--- | :--- | :---: | :---: | : |
| **Target (Retail)** | Retail target disassembly | **0** (90w / `0x30`) | **0** (90w / `0x30`) | `magic=f4`, `sx=f2`, `int=f3` -> `fsubs f0, f3, f4`, `fmuls f0, f0, f1`, `fmuls f0, f0, f2` |
| **Baseline (J0)** | Canonical local copy `mVec2_c speed; if ...` | **5** (90w / `0x30`) | **5** (90w / `0x30`) | `magic=f3`, `sx=f4`, `int=f2` -> `fsubs f0, f2, f3`, `fmuls f0, f0, f1`, `fmuls f0, f4, f0` |
| **J1_const_ref** | `const mVec2_c &speed = (flag != 0) ? ... : ...;` | 44 (84w / -6w) | 44 (84w / -6w) | `magic=f2`, `sx=None` (no stack copy), `fsubs f0, f0, f2`, `fmuls f0, f0, f1`, `fmuls f0, f3, f0` |
| **J2_const_ptr** | `const mVec2_c *speed = (flag != 0) ? ... : ...;` | 44 (84w / -6w) | 44 (84w / -6w) | `magic=f2`, `sx=None` (no stack copy), `fsubs f0, f0, f2`, `fmuls f0, f0, f1`, `fmuls f0, f3, f0` |
| **J3_ptr_if_else** | `const mVec2_c *speed; if (flag != 0) ...;` | 44 (84w / -6w) | 44 (84w / -6w) | `magic=f2`, `sx=None` (no stack copy), `fsubs f0, f0, f2`, `fmuls f0, f0, f1`, `fmuls f0, f3, f0` |
| **J4_decl_order** | `float sx; float muki; float rate = ...;` (split declaration) | **5** (90w / `0x30`) | **5** (90w / `0x30`) | `magic=f3`, `sx=f4`, `int=f2` -> `fsubs f0, f2, f3`, `fmuls f0, f0, f1`, `fmuls f0, f4, f0` |
| **M1_sx_first_prod** | `mSpeed.x = sx * (muki * rate);` | **5** (90w / `0x30`) | **5** (90w / `0x30`) | `magic=f3`, `sx=f4`, `int=f2` -> `fsubs f0, f2, f3`, `fmuls f0, f0, f1`, `fmuls f0, f4, f0` |
| **M7_compound_sx** | `mSpeed.x = muki * rate; mSpeed.x *= sx;` | **5** (90w / `0x30`) | **5** (90w / `0x30`) | `magic=f3`, `sx=f4`, `int=f2` -> `fsubs f0, f2, f3`, `fmuls f0, f0, f1`, `fmuls f0, f0, f4` |
| **T1_sy_first** | `f32 sy = speed.y; f32 sx = speed.x; ...` | 7 (90w / `0x30`) | 7 (90w / `0x30`) | `magic=f2`, `sx=f4`, `int=f0` -> `fsubs f0, f0, f2`, `fmuls f0, f0, f1`, `fmuls f0, f4, f0` |
| **T5_speed_direct** | `mSpeed.x = (muki * rate) * speed.x;` | 8 (90w / `0x30`) | 8 (90w / `0x30`) | `magic=f3`, `sx=f0`, `int=f2` -> `fsubs f2, f2, f3`, `fmuls f1, f2, f1`, `fmuls f0, f0, f1` |

**Takeaway on Speed Provenance**:
Passing `speed` by `const reference` or `const pointer` completely strips the 2 stack stores `stfs f1, 0x10(r1)` and `stfs f0, 0x14(r1)`, shrinking the functions by 6 words (84w vs 90w target). The retail binary indisputably uses a copied local `mVec2_c speed;`, which creates the `0x10(r1)` and `0x14(r1)` stack slots.

---

## 4. GAINED & LOST Sections

### GAINED Functions (0 Gained)
No new functions reached 0-diff byte-identity in Round 34. The primary draft `scratch/gemini_round24/d_enemy_toride_kokoopa.cpp` preserves the canonical 248 matching functions, the verified `.rodata` compound literal, and the canonical 5-diff Jump twins.

### LOST Functions (0 Lost)
Zero functions regressed or left the matched set in Round 34.

### Explicit Artifact-Matched Pair Re-Check:
All 4 artifact-matched functions were re-verified against the freshly compiled primary object `d_enemy_toride_kokoopa.o` and remain 100% matched under the union gate / naming-artifact rule:
1. **`__sinit_\\d_enemy_toride_kokoopa_cpp` (5,784 B / 1,446 insns / frame `0x420`):**
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

## 5. Constant Pool Verification (`poolcheck.py`)

```
""")
parts.append(poolcheck)
parts.append("""
```

---

## 6. Full Unit Function Diff Log (`fndiff.py --all`)

```
""")
parts.append(fndiff_all)
parts.append("""
```

---

## 7. NOT REACHED

None. All three numbered items in the Round 34 work order were investigated and executed in full.
""")

out_path = os.path.join(ROOT, 'GEMINI_RESPONSE.md')
with open(out_path, 'w', encoding='utf-8') as f:
    f.write(''.join(parts))

print('GEMINI_RESPONSE.md written, size:', os.path.getsize(out_path))

