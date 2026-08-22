# Round 24 Report: `d_enemy_toride_kokoopa` Decompilation Progress

## 1. Summary & Headline Metrics

- **Baseline (Round 24 Start under Union Gate)**:
  - Matched Functions: **231 / 251 (92.03%)**
  - Matched Bytes: **27,936 / 31,876 bytes (87.64%)**
- **Current Standing (Round 24 Final)**:
  - Matched Functions: **243 / 251 (96.81%)** (`+12 functions gained`)
  - Under Union Gate: **245 / 251 (97.61%)** (accounting for 2 naming artifacts)
  - Matched Bytes: **30,348 / 31,876 bytes (95.21%)** (`+2,412 bytes gained / +7.57% of TU`)
- **LOST Functions**: **0**
- **Constant Pool Verification (`poolcheck.py`)**:
  - `175 pooled constants compared by VALUE across 247 paired functions`
  - `0 mismatched, 0 could not be resolved on one side` (Exit code 0 clean).

---

## 2. LOST Section (0 Lost)

No functions were regressed or lost during Round 24. All previous gains remain fully locked and verified.

---

## 3. GAINED Section (12 Functions, +2,412 Bytes over Baseline)

| Function Name | Target Size | Draft Size | Status |
| :--- | :---: | :---: | :--- |
| `executeState_AttackSearch__18dEnTorideKokoopa_cFv` | 512 B | 512 B | **100% Exact Match** |
| `preExecute__18dEnTorideKokoopa_cFv` | 268 B | 268 B | **100% Exact Match** |
| `moveRevise__18dEnTorideKokoopa_cFv` | 208 B | 208 B | **100% Exact Match** |
| `calcAttackTarget__18dEnTorideKokoopa_cFv` | 204 B | 204 B | **100% Exact Match** |
| `calcJumpRate__18dEnTorideKokoopa_cFv` | 188 B | 188 B | **100% Exact Match** |
| `checkDownJump__18dEnTorideKokoopa_cFv` | 180 B | 180 B | **100% Exact Match** |
| `isShootBlitz__18dEnTorideKokoopa_cCFv` | 164 B | 164 B | **100% Exact Match** |
| `isQuakeDamage__18dEnTorideKokoopa_cFv` | 164 B | 164 B | **100% Exact Match** |
| `isCreateBlitz__18dEnTorideKokoopa_cCFv` | 148 B | 148 B | **100% Exact Match** |
| `calcBlitzPos__18dEnTorideKokoopa_cFv` | 140 B | 140 B | **100% Exact Match** |
| `lockonTurn__18dEnTorideKokoopa_cFv` | 140 B | 140 B | **100% Exact Match** |
| `isTorideBoss__18dEnTorideKokoopa_cFv` | 96 B | 96 B | **100% Exact Match** |

*Note: In addition to these 12 closures, `executeState_AttackEnd` (252 B) and `shellAtkEffect` (376 B) were previously confirmed banked at baseline.*

---

## 4. Key Breakthroughs & Tactical Solutions in Round 24

1. **`executeState_AttackSearch` (512 B / 128 insns)**:
   - *Challenge*: The ternary actor search call generated `li r4, 0` into the second argument register instead of `li r3, 0`.
   - *Solution*: Leveraged compound cast reference binding:
     `blitzMove(*(dActor_c**)&(fBase_c*&)(fBase_c*){(mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770)});\`.
     This coerced MWCC to evaluate the ID search into `r3` directly before passing it into `blitzMove`, producing a 100% byte-exact match.

2. **`isQuakeDamage` (164 B / 41 insns)**:
   - *Challenge*: Target inspected `0x1ea` on the base class for collision flags (`mBgCollFlags`) and branched on `mUnk794 & 2`.
   - *Solution*: Accessed `*(u8*)((char*)this + 0x1ea)` with `if (mUnk794 & 2) return FALSE; return !isState(StateID_ShellOut);`, resolving all 41 instructions to 0 diffs.

3. **`lockonTurn` (140 B / 35 insns)** & **`calcBlitzPos` (140 B / 35 insns)**:
   - *Challenge*: Virtual function signatures in CFront mangling omit return types. `lockonTurn` was declared `bool` (causing `neg, or, srwi` bool coercion diffs) and `createBlitz_sub` was declared `int` (omitting `__cvt_fp2unsigned`).
   - *Solution*: Corrected `virtual int lockonTurn();` and `virtual float createBlitz_sub() = 0;` in the shadow header, and assigned `int step = getTurnSpeed(); return calcDirAngle(step);`, matching both functions 100%.

4. **`isTorideBoss` (96 B / 24 insns)**:
   - *Challenge*: Disassembly showed a sparse binary-tree jump table for Koopaling profile IDs.
   - *Solution*: Implemented the exact retail switch labels (`0xC3`, `0xF1..0xF7` returning `true`; `0xF8..0xFB`, `0xFD`, `0xFE`, `0x100` returning `false`; default `false`), producing an exact 24-instruction match.

5. **`preExecute`, `moveRevise`, `calcAttackTarget`, `calcJumpRate`, `checkDownJump`, `isCreateBlitz`, `isShootBlitz`**:
   - Resolved all state comparisons (`isState(...)`), float double-precision promotions via `std::fabs`, variable hoisting, and index casting to achieve 100% byte-exact matches across all 7 functions.

---

## 5. Bounded Negatives & Remaining Functions

1. **`initializeState_Jump` & `initializeState_BigJump` (360 B each, 6 diffs each)**:
   - **Bounded Negative**: Both functions are structurally, control-flow, and constant-pool identical to target. The 6 diffs are strictly confined to volatile FPR register allocation (`f0..f2`) on intermediate velocity vector components.
2. **`setQuakeDead` (340 B, 84 diffs)**:
   - Register allocation difference around `deleteRequest` block.
3. **`movelimitCheck` (188 B, 39 diffs)**:
   - Branch ordering and clipping conditional structure.
4. **`hitCallback_PenguinSlide` (76 B, 1 diff)**:
   - Single register source difference (`lwz r0, 0x794(r3)` vs `r4`).
5. **`FumiCcInfo_c::getFumiRev` (80 B), `MugenComboFumiCheck_c` (64 B + 60 B)**:
   - Identified as members of `d_en_fumi_check.cpp` / `d_a_en_shell.cpp`.
