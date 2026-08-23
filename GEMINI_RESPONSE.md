# Round 26 Report: `d_enemy_toride_kokoopa` Decompilation Progress

## 1. Summary & Headline Metrics

- **Baseline (Round 25 Corrected, with `__sinit` broken)**:
  - Matched Functions: **246 / 251 (98.01%)**
  - Matched Bytes: **24,956 / 31,876 bytes (78.29%)**
- **Current Standing (Round 26 Final)**:
  - Matched Functions: **248 / 251 (98.80%)** (`+2 functions gained over corrected baseline`)
  - Matched Bytes: **30,816 / 31,876 bytes (96.67%)** (`+5,860 bytes gained`)
- **LOST Functions**: **0** (Zero regressions across both raw matches and artifact-matched functions).
- **Constant Pool Verification (`poolcheck.py`)**:
  - `177 pooled constants compared by VALUE across 250 paired functions`
  - `0 mismatched, 0 could not be resolved on one side` (Exit code 0 clean).

---

## 2. LOST Section (0 Lost)

No functions regressed or left the matched set in Round 26 across both the raw match set and the artifact-matched set.

### Explicit Artifact-Matched Functions Check:
1. **`__sinit_\d_enemy_toride_kokoopa_cpp` (5,784 B)**:
   - Raw Byte Diffs: **0**
   - Canonical Diffs: **4** (state ID template instantiation naming artifacts)
   - Status: **MATCHED** (under union gate / naming-artifact rule)
2. **`executeState_ShellAtk_St__18dEnTorideKokoopa_cFv` (612 B)**:
   - Raw Byte Diffs: **0**
   - Canonical Diffs: **2** (state ID template instantiation naming artifacts)
   - Status: **MATCHED** (under union gate / naming-artifact rule)

---

## 3. GAINED Section (2 Functions, +5,860 Bytes over Corrected Baseline)

| Function Name | Target Size | Draft Size | Closure Mechanism / Root Cause |
| :--- | :---: | :---: | :--- |
| `__sinit_\d_enemy_toride_kokoopa_cpp` | 5,784 B | 5,784 B | **Recovered / Matched** (Reduced `g_padData` from 128 B to 112 B; offsets in `.data` restored `__vt__18dEnTorideKokoopa_c` to `0x80314360`, returning `__sinit` from 196 diffs to 4 diffs / 0 raw diffs). |
| `hitCallback_PenguinSlide__18dEnTorideKokoopa_cFP5dCc_cP5dCc_c` | 76 B | 76 B | **100% Byte-Exact Match** (Reordered evaluation so `mUnk794` is read directly from `r3` before argument setup overwrites it, eliminating the `r4` copy-propagation diff). |

---

## 4. Work Order Item Analysis

### Item 1: Recalibrate Pad & Restore `__sinit` (+5,784 Bytes)
- **Root Cause of Regression**:
  In Round 25, `KokoopaSpFumiCheck_c` was introduced with virtual methods, causing CodeWarrior to emit `__vt__20KokoopaSpFumiCheck_c` (size `0x10` = 16 bytes) into `.data` immediately preceding `__vt__18dEnTorideKokoopa_c`. This displaced `__vt__18dEnTorideKokoopa_c` and subsequent state ID descriptor vtables by +16 bytes (`0x80314360` -> `0x80314370`), disrupting the 49 static state registrations inside `__sinit`.
- **Arithmetic & Resolution**:
  - `g_padData` previously = 128 bytes (`0x80`).
  - Size of new preceding vtable (`__vt__20KokoopaSpFumiCheck_c`) = 16 bytes (`0x10`).
  - Recalibrated `g_padData` size = `128 - 16 = 112` bytes (`0x70`).
- **Result**:
  - `__vt__18dEnTorideKokoopa_c` returned to exact address `0x80314360`.
  - `__sinit` returned from 196 diffs to 4 canonical diffs / 0 raw byte diffs (100% matched under union gate).

---

### Item 2: `hitCallback_PenguinSlide` (+76 Bytes)
- **Previous Diff**: 1 instruction diff (`lwz r0, 0x794(r4)` vs retail `lwz r0, 0x794(r3)`).
- **Resolution**:
  CodeWarrior copy propagation substituted `r4` for `this` when `setDamage`'s first parameter was evaluated before reading `mUnk794`. Assigning the owner pointer and evaluating `(mUnk794 & 2)` beforehand compelled MWCC to read `0x794(r3)` directly from `r3`:
  ```cpp
  bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
      daPlBase_c *pl = (daPlBase_c*)otherCc->getOwner();
      daPlBase_c::DamageType_e dmg = (daPlBase_c::DamageType_e)3;
      if (mUnk794 & 2) dmg = (daPlBase_c::DamageType_e)2;
      pl->setDamage(this, dmg);
      return true;
  }
  ```
- **Result**: 0 diffs, 100% exact byte match.

---

### Item 3: `initializeState_Jump` / `initializeState_BigJump` Register File Inspection
- **Prompt Query**: *State which register file the diffs are in, in your report, before drawing any conclusion. If `f0`..`f13`, they are volatile, the lever does not apply, `AGENT_CONTEXT.md` records that as a bounded negative, and you should say so and stop. If `f14`..`f31`, they are callee-saved and the lever is proven...*
- **Register File Answer**:
  - The diffs are strictly in registers **`f0`, `f1`, `f2`, `f3`, and `f4`**.
  - These belong entirely to the **VOLATILE floating-point register file (`f0`..`f13`)**.
  - **Zero callee-saved FPRs (`f14`..`f31`)** are saved or used on either side.
  - **Prologues**: Both target and draft execute `stwu r1, -0x30(r1); mflr r0; stw r0, 0x34(r1); stw r31, 0x2c(r1); stw r30, 0x28(r1);` (0 FPRs saved, stack size `0x30`).
  - **Epilogues**: Both target and draft execute `lwz r31, 0x2c(r1); lwz r30, 0x28(r1); mtlr r0; addi r1, r1, 0x30; blr;` (0 FPRs restored).
- **Conclusion**:
  - Because no callee-saved FPRs are involved, the spurious FPR save lever does not apply.
  - This is confirmed as a **bounded negative** representing volatile scratch register DAG scheduling permutation in MWCC.

---

### Item 4: `setQuakeDead` (340 B Target / 352 B Draft, 84 diffs)
- **Disassembly Diagnosis**:
  - **Target** (340 B / 85 instructions): Saves only `r30` and `r31` with stack frame `0x30`. In target, `searchBaseByID` is called directly, and its return value in volatile `r3` is tested (`cmpwi r3, 0; beq ...; bl deleteRequest`) without persisting `r3` across function calls.
  - **Draft** (352 B / 88 instructions): MWCC's register allocator hoists `0` across `UnKnownScoreSet` and allocates non-volatile register `r29` for the `searchBaseByID` ternary / `0` constant, expanding the stack frame to `0x40` (88 instructions / 352 bytes).
  - When `mUnk770` is guarded via non-ternary branching (`if (mUnk770 != 0)`), stack size is `0x30` and all instructions match except the 4-instruction ternary branching sequence (`bne` / `li r3, 0` / `b` vs `beq` / `bl searchBaseByID`).

---

## 5. Summary of Remaining 3 Unmatched Functions (1,060 Bytes Total)

1. **`initializeState_Jump__18dEnTorideKokoopa_cFv` (360 B, 6 diffs)**:
   - Volatile FPR scratch allocation (`f0..f4`). Bounded negative.
2. **`initializeState_BigJump__18dEnTorideKokoopa_cFv` (360 B, 6 diffs)**:
   - Volatile FPR scratch allocation (`f0..f4`). Bounded negative.
3. **`setQuakeDead__18dEnTorideKokoopa_cFv` (340 B, 84 diffs)**:
   - Spurious `r29` allocation / stack frame `0x40` vs `0x30`.

---

## 6. `poolcheck.py` Output

```
177 pooled constants compared by VALUE across 250 paired functions
0 mismatched, 0 could not be resolved on one side
(248 pair(s) value-checked; 16 reference(s) skipped as the same named symbol on both sides; 381 float load(s) seen; 1 pair(s) skipped on length)
COVERAGE: 248 of 488 target function(s) value-checked; 240 were not checked at all (unpaired, length-mismatched, or already differing).
```
Exit code: 0 (clean).
