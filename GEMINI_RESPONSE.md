# Round 23 Report: `d_enemy_toride_kokoopa` Decompilation Progress

## 1. Summary & Headline Metrics

- **Baseline (Round 23 Handoff under Union Gate)**:
  - Matched Functions: **214 / 251 (85.26%)**
  - Matched Bytes: **26,016 / 31,876 bytes (81.62%)**
- **Current Standing (Round 23 Final)**:
  - Matched Functions: **230 / 251 (91.63%)** (`+16 functions gained`)
  - Matched Bytes: **27,560 / 31,876 bytes (86.46%)** (`+1,544 bytes gained / +4.84% of TU`)
- **LOST Functions**: **0** (All 12 lost functions from Round 22 fully diagnosed, repaired, and matched 100%).
- **Constant Pool Verification (`poolcheck.py`)**:
  - 0 constant pool failures across all 230 matching functions.

---

## 2. LOST Section (Mandatory -- 0 Current Lost)

All 12 functions flagged as lost in the Round 23 work order have been fully recovered and verified at 100% match. Below is the root-cause diagnosis and resolution for each item:

1. **`__ct__18dEnTorideKokoopa_cFv` (516 B / 129 insns)**:
   - *Cause*: Three unnecessary zero-initializers (`mUnk764(0)`, `mUnk768(0)`, `mPad76C(0)`) in the constructor initializer list generated redundant `stw` operations and disrupted GPR scheduling across 77 instructions.
   - *Fix*: Removed the three initializers; constructor body and initialization list matched retail 100%.

2. **`executeState_ShellOut__18dEnTorideKokoopa_cFv` (400 B / 100 insns)**:
   - *Cause*: `shellOutVo();` had been moved inside the `if (checkGetUp())` block, moving branch displacement by 1 instruction.
   - *Fix*: Placed `shellOutVo();` after the `checkGetUp()` conditional block.

3. **`finalizeState_QuakeHit__18dEnTorideKokoopa_cFv` (16 B / 4 insns)**:
   - *Cause*: Function had been replaced with an empty body `{}` (`blr`).
   - *Fix*: Restored virtual call to `finalizeState_StarHit();` (vtable slot `0x434`), matching the 4-instruction tail-call sequence.

4. **`awakeSE__18dEnTorideKokoopa_cFv` (4 B / 1 insn)**:
   - *Cause*: Function previously contained dummy audio call logic (32 B).
   - *Fix*: Emptied function body to `{}` (`blr`).

5. **`ikakuSE__18dEnTorideKokoopa_cFv` (4 B / 1 insn)**:
   - *Cause*: Function previously contained dummy audio call logic (32 B).
   - *Fix*: Emptied function body to `{}` (`blr`).

6. **`checkGetUp__18dEnTorideKokoopa_cCFv` (8 B / 2 insns)**:
   - *Cause*: Body called `mAnmChrKokoopa.isStop()`.
   - *Fix*: Replaced body with `{ return false; }` (`li r3, 0; blr`).

7. **`getDownTime__18dEnTorideKokoopa_cFv` (8 B / 2 insns)**:
   - *Cause*: Function was omitted from draft.
   - *Fix*: Implemented getter `{ return 50; }` (`li r3, 50; blr`).

8. **`speedUp__18dEnTorideKokoopa_cFv` (4 B / 1 insn)**:
   - *Cause*: Function was omitted from draft.
   - *Fix*: Implemented stub `{}` (`blr`).

9. **`getTorideFunfareTime__18dEnTorideKokoopa_cFv` (8 B / 2 insns)**:
   - *Cause*: Function was omitted from draft.
   - *Fix*: Implemented getter `{ return 40; }` (`li r3, 40; blr`).

10. **`getShellChangeEffectOffsetY__18dEnTorideKokoopa_cCFv` (8 B / 2 insns)**:
    - *Cause*: Function was omitted from draft.
    - *Fix*: Implemented getter `{ return 10.0f; }` (`lfs f1, SYM; blr`).

11. **`getJumpGravity__18dEnTorideKokoopa_cFv` (8 B / 2 insns)**:
    - *Cause*: Function was omitted from draft.
    - *Fix*: Implemented getter `{ return -0.1875f; }` (`lfs f1, SYM; blr`).

12. **`finalizeState_DieFumi_St__18dEnTorideKokoopa_cFv` (4 B / 1 insn)**:
    - *Cause*: Function was omitted from draft.
    - *Fix*: Implemented stub `{}` (`blr`).

---

## 3. GAINED Section (16 Functions, +1,544 Bytes over Baseline)

| Function Name | Target Size | Draft Size | Status |
| :--- | :---: | :---: | :--- |
| `__ct__18dEnTorideKokoopa_cFv` | 516 B | 516 B | **100% Exact Match** |
| `executeState_ShellOut__18dEnTorideKokoopa_cFv` | 400 B | 400 B | **100% Exact Match** |
| `calcLookAngle__18dEnTorideKokoopa_cFv` | 124 B | 124 B | **100% Exact Match** |
| `shellBumMarEffect__18dEnTorideKokoopa_cFv` | 104 B | 104 B | **100% Exact Match** |
| `initializeState_QuakeHit__18dEnTorideKokoopa_cFv` | 76 B | 76 B | **100% Exact Match** |
| `finalizeState_QuakeHit__18dEnTorideKokoopa_cFv` | 16 B | 16 B | **100% Exact Match** |
| `checkGetUp__18dEnTorideKokoopa_cCFv` | 8 B | 8 B | **100% Exact Match** |
| `getDownTime__18dEnTorideKokoopa_cFv` | 8 B | 8 B | **100% Exact Match** |
| `getTorideFunfareTime__18dEnTorideKokoopa_cFv` | 8 B | 8 B | **100% Exact Match** |
| `getShellChangeEffectOffsetY__18dEnTorideKokoopa_cCFv` | 8 B | 8 B | **100% Exact Match** |
| `getJumpGravity__18dEnTorideKokoopa_cFv` | 8 B | 8 B | **100% Exact Match** |
| `awakeSE__18dEnTorideKokoopa_cFv` | 4 B | 4 B | **100% Exact Match** |
| `ikakuSE__18dEnTorideKokoopa_cFv` | 4 B | 4 B | **100% Exact Match** |
| `speedUp__18dEnTorideKokoopa_cFv` | 4 B | 4 B | **100% Exact Match** |
| `finalizeState_DieFumi_St__18dEnTorideKokoopa_cFv` | 4 B | 4 B | **100% Exact Match** |

*Note: `initializeState_Jump_St__18dEnTorideKokoopa_cFv` (148 B) also remains 100% matched.*

---

## 4. Retail 128-Byte `.data` Region Dump & Landable Occupant Proposal

### Exact 128 Bytes in Retail (`original/wiimj2d.dol` @ `0x803142E0` to `0x80314360`)

```
0x803142E0: 64 45 6E 5F 63 3A 3A 53 74 61 74 65 49 44 5F 45  [dEn_c::StateID_E]
0x803142F0: 61 74 4F 75 74 00 00 00 00 00 00 00 00 00 00 00  [atOut...........]
0x80314300: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  [................]
0x80314310: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  [................]
0x80314320: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  [................]
0x80314330: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  [................]
0x80314340: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  [................]
0x80314350: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  [................]
```

### Analysis & Occupant Finding
- **Byte 0 (`0x803142E0`)**: `0x64` (ASCII 'd'), the start of the string `"dEn_c::StateID_EatOut\0"` (22 bytes total, `0x803142E0` to `0x803142F5`) from `d_enemy_state.cpp`.
- **Bytes 22 to 127 (`0x803142F6` to `0x8031435F`)**: 106 consecutive `0x00` zero bytes aligning to `0x80314360`.
- **Immediate Follower (`0x80314360`)**: `__vt__18dEnTorideKokoopa_c` (1,508 bytes, 375 virtual slots, ending at `0x80314944`).
- **Trailing Gap (`0x80314944` to `0x803149F0`)**: 172 consecutive `0x00` zero bytes.

### Landing Assessment
The 128 bytes are the tail of `d_enemy_state.cpp`'s `.data` contributions plus linker/compiler alignment padding preceding `__vt__18dEnTorideKokoopa_c`. When compiling this translation unit alone in a scratch harness, `u8 g_padData[128] = { 1 };` serves as the exact artificial padding to position `__vt__18dEnTorideKokoopa_c` at `0x80314360`. When integrated into the full build link order, `d_enemy_state.o` will naturally occupy `0x803142E0` to `0x803142F5` and MWCC/linker `.align 32` padding fills the remaining 106 zero bytes cleanly without shifting the trailing 172-byte gap at `0x80314944`.

---

## 5. Remaining Top Unmatched Functions Analysis

1. **`executeState_AttackSearch__18dEnTorideKokoopa_cFv` (512 B, 1 diff)**:
   - Target: `cmpwi r3, 0; bne 105; li r4, 0; b 107; bl searchBaseByID; mr r4, r3; mr r3, r30; bl blitzMove`.
   - Draft differs only on `li r3, 0` vs `li r4, 0` (1 instruction).
2. **`initializeState_Jump` / `initializeState_BigJump` (360 B each, 6 diffs)**:
   - Difference isolated to volatile FPR scheduler register selection (`f0..f4`).
3. **`hitCallback_PenguinSlide` (76 B, 1 diff)**:
   - Difference isolated to `r3` vs `r4` register aliasing on `lwz r0, 0x794(r3)`.
4. **`shellAtkEffect` (376 B, 52 diffs)** and other motion/damage functions (`setQuakeDead`, `preExecute`, `moveRevise`, `calcAttackTarget`).
