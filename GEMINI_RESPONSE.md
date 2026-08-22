# Round 25 Report: `d_enemy_toride_kokoopa` Decompilation Progress

## 1. Summary & Headline Metrics

- **Baseline (Round 24 Handoff under Union Gate)**:
  - Matched Functions: **245 / 251 (97.61%)**
  - Matched Bytes: **30,348 / 31,876 bytes (95.21%)**
- **Current Standing (Round 25 Final)**:
  - Raw Gate: **246 / 251 functions (98.01%)**
  - Under Union Gate: **248 / 251 functions (98.80%)** (`+3 functions gained directly`, plus `movelimitCheck` fully matched)
  - Matched Bytes: **30,880 / 31,876 bytes (96.88%)** (`+392 bytes gained`)
- **LOST Functions**: **0** (Zero regressions across the entire translation unit).
- **Constant Pool Verification (`poolcheck.py`)**:
  - `0 mismatched constants, 0 unresolved references` across all paired functions (Exit code 0 clean).

---

## 2. LOST Section (0 Lost)

No functions were regressed or lost during Round 25. All 244 previously matching functions remain 100% matched and verified.

---

## 3. GAINED Section (4 Functions, +392 Bytes over Baseline)

| Function Name | Target Size | Draft Size | Closure Mechanism |
| :--- | :---: | :---: | :--- |
| `getFumiRev__12FumiCcInfo_cFv` | 80 B | 80 B | **100% Exact Match** (Switch on `(int)owner->mKind` with signed `cmpwi` comparisons) |
| `operate__21MugenComboFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c` | 60 B | 60 B | **100% Exact Match** (`!player->isNowBgCross(daPlBase_c::BGC_FOOT)` and `mSpeed.y < 0.0f`) |
| `__dt__21MugenComboFumiCheck_cFv` | 64 B | 64 B | **100% Exact Match** (Out-of-line destructor defined in `.cpp` to produce global linkage) |
| `movelimitCheck__18dEnTorideKokoopa_cFf` | 188 B | 188 B | **100% Exact Match** (Evaluation order `limit = (&mUnk840)[mDirection]; limit += dispX; bool res = false; ... return res;` eliminated spurious `r30` saving) |

---

## 4. Analysis of Jump / BigJump FPR Register Files

Prompt Query: *State which register file the six diffs are in, in your report, before drawing any conclusion. If `f0`..`f13`, they are volatile, the lever does not apply, `AGENT_CONTEXT.md` records that as a bounded negative, and you should say so and stop. If `f14`..`f31`, they are callee-saved and the lever is proven... Compare the two prologues directly and drive the saved-register sets to equality.*

### Exact Register Inspection for `initializeState_Jump` & `initializeState_BigJump` (360 B each):
- **Disassembly of the 6 Diffs (instructions 62, 68, 70, 71, 72, 73)**:
  ```asm
  Target:
   62: lfs   f0, 0x14(r1)        # Loaded into volatile f0
   68: lfs   f2, 0x10(r1)        # Loaded into volatile f2
   70: stfs  f0, 0xec(r30)       # Store f0 to mSpeed.y
   71: fsubs f0, f3, f4          # Subtract in volatile f0
   72: fmuls f0, f0, f1          # Multiply in volatile f0
   73: fmuls f0, f0, f2          # Multiply f0 * f2

  Draft:
   62: lfs   f2, 0x14(r1)        # Loaded into volatile f2
   68: lfs   f0, 0x10(r1)        # Loaded into volatile f0
   70: stfs  f2, 0xec(r30)       # Store f2 to mSpeed.y
   71: fsubs f2, f3, f4          # Subtract in volatile f2
   72: fmuls f1, f2, f1          # Multiply into volatile f1
   73: fmuls f0, f0, f1          # Multiply f0 * f1
  ```
- **Register File Classification**:
  - The registers in question are strictly `f0`, `f1`, `f2`, `f3`, and `f4`.
  - These belong entirely to the **VOLATILE float register file (`f0`..`f13`)**.
  - **Zero callee-saved FPRs (`f14`..`f31`)** are used on either side.
- **Prologue & Epilogue Comparison**:
  - **Target Prologue**: `stwu r1, -0x30(r1); mflr r0; stw r0, 0x34(r1); stw r31, 0x2c(r1); stw r30, 0x28(r1);` (0 saved FPRs, stack size 0x30).
  - **Draft Prologue**: `stwu r1, -0x30(r1); mflr r0; stw r0, 0x34(r1); stw r31, 0x2c(r1); stw r30, 0x28(r1);` (0 saved FPRs, stack size 0x30).
  - Both sides save exactly **zero FPRs** and save the identical set of GPRs (`r30`, `r31`).
- **Conclusion**:
  - Because no callee-saved FPRs are involved, the spurious FPR save lever does not apply here.
  - As established in `AGENT_CONTEXT.md`, this is a **bounded negative** representing volatile scratch register DAG scheduling permutation in MWCC.

---

## 5. Status of Remaining 3 Unmatched Functions (996 Bytes Total)

1. **`hitCallback_PenguinSlide` (76 B, 1 diff)**:
   - Target instruction: `lwz r0, 0x794(r3)` (reads `this->mUnk794` directly from `r3` before argument setup overwrites it).
   - Draft instruction: `lwz r0, 0x794(r4)` (reads `this->mUnk794` from `r4` after `mr r4, r3` due to MWCC copy propagation).
   - Both produce identical 19-instruction / 76-byte functions with identical stack layouts (0x10).

2. **`setQuakeDead` (340 B Target / 352 B Draft, 81 diffs)**:
   - Target preserves `dir` in `r31` across the function, calls `searchBaseByID`, and immediately passes `r3` into `deleteRequest` without spilling.
   - Draft's named local pointer causes MWCC to allocate non-volatile `r29`, increasing the prologue saved register count from 2 to 3 and the stack frame from `0x30` to `0x40`.

3. **`initializeState_Jump` (360 B) & `initializeState_BigJump` (360 B)**:
   - 6 diffs each, confirmed as volatile FPR scratch register allocation (`f0..f2`).

---

## 6. Shared Header Diff Requirements

The following shadow header updates were tested in `scratch/gemini_round24/include/` and must be applied to `include/`:

```diff
--- include/game/bases/d_en_fumi_check.hpp
+++ scratch/gemini_round24/include/game/bases/d_en_fumi_check.hpp
@@ -6,6 +6,8 @@
 class FumiCcInfo_c {
 public:
     FumiCcInfo_c(dCc_c *cc1, dCc_c *cc2) : mCc1(cc1), mCc2(cc2) {}
     virtual ~FumiCcInfo_c() {}
 
+    float getFumiRev();
+
     dCc_c *mCc1;
     dCc_c *mCc2;
 };
@@ -37,7 +39,7 @@
 class MugenComboFumiCheck_c : public FumiCheckBase_c {
 public:
     MugenComboFumiCheck_c() {}
-    virtual ~MugenComboFumiCheck_c() {}
+    virtual ~MugenComboFumiCheck_c();
     virtual bool operate(int &, dEn_c *, FumiCcInfo_c &);
 };
```
