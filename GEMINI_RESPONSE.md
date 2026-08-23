# Round 27 Report: `d_enemy_toride_kokoopa` Decompilation & Landing Assessment

## 1. Summary & Headline Metrics

- **Baseline (Round 26 Final / Verified Baseline)**:
  - Matched Functions: **248 / 251 (98.80%)**
  - Matched Bytes: **30,816 / 31,876 bytes (96.67%)**
- **Current Standing (Round 27)**:
  - Matched Functions: **248 / 251 (98.80%)**
  - Matched Bytes: **30,816 / 31,876 bytes (96.67%)**
- **GAINED Functions**: **0**
- **LOST Functions**: **0** (Zero regressions across both raw matches and artifact-matched functions).
- **Constant Pool Verification (`poolcheck.py`)**:
  - `177 pooled constants compared by VALUE across 250 paired functions`
  - `0 mismatched, 0 could not be resolved on one side` (Exit code 0 clean).

---

## 2. LOST Section (0 Lost)

No functions regressed or left the matched set in Round 27.

### Explicit Artifact-Matched Pair Check:
1. **`__sinit_\d_enemy_toride_kokoopa_cpp` (5,784 B)**:
   - Raw Byte Diffs: **0**
   - Canonical Diffs: **4** (state ID template instantiation naming artifacts)
   - Status: **MATCHED** (under union gate / naming-artifact rule)
2. **`executeState_ShellAtk_St__18dEnTorideKokoopa_cFv` (612 B)**:
   - Raw Byte Diffs: **0**
   - Canonical Diffs: **2** (state ID template instantiation naming artifacts)
   - Status: **MATCHED** (under union gate / naming-artifact rule)

---

## 3. Work Order Item 1: `setQuakeDead` Analysis & Experimentation

- **Baseline**: 340 B Target / 352 B Draft (84 diffs)
- **Root Cause & Diagnosis**:
  - **Target** (340 B / 85 instructions):
    - Prologue saves strictly `r30` (`this`) and `r31` (`dir` from `getPl_LRflag`), stack frame `0x30` (`stwu r1, -0x30(r1)`).
    - Stores `mUnk792 = 0` and `mUnk790 = 0` using volatile register `r0` (`li r0, 0; sth r0, 0x792(r30); sth r0, 0x790(r30)`).
    - Calls `UnKnownScoreSet`.
    - Directly executes the ternary: `lwz r3, 0x770(r30); cmpwi r3, 0; bne .L_call; li r3, 0; b .L_check; .L_call: bl searchBaseByID; .L_check: cmpwi r3, 0; beq .L_skip; bl deleteRequest`.
    - Crucially, `r3` is tested directly without any copy (`mr`) into a non-volatile register, and `deleteRequest()` takes `r3` directly as its receiver (`this`).
  - **Draft with Standard Ternary** (352 B / 88 instructions):
    - CodeWarrior's global common subexpression elimination (CSE) detects `0` before `UnKnownScoreSet` (`mUnk792/mUnk790`) and `0` in the ternary arm `(mUnk770 == 0 ? 0 : searchBaseByID)`.
    - MWCC hoists constant `0` across `UnKnownScoreSet` into non-volatile register `r29`.
    - At the ternary merge phi-node, MWCC assigns `r29` to hold `base`, emitting `mr r29, r3; cmpwi r29, 0; beq; mr r3, r29; bl deleteRequest`.
    - This forces `r29` to be preserved across the prologue/epilogue, expanding the stack frame to `0x40` (88 instructions / 84 diffs).
  - **Draft with Direct `if (mUnk770 != 0)` Guard** (340 B / 85 instructions):
    - Eliminating the `0` constant from the ternary stops `0` from surviving `UnKnownScoreSet`.
    - Stack frame drops to `0x30` with `r29` completely eliminated from prologue/epilogue.
    - Matches 81 of 85 instructions byte-for-byte; only the 4 branch-dispatch instructions differ (`beq; bl searchBaseByID; cmpwi r3, 0; beq; bl deleteRequest` vs retail's `bne; li r3, 0; b; bl searchBaseByID; cmpwi r3, 0; beq; bl deleteRequest`).
  - **Experiments Run**:
    - Tested 20+ variations (comma operator, inline helpers, local lambdas, volatile casts, type casts, nested conditionals, assignment inside conditionals).
    - Result: Whenever the C++ syntax produces the authentic 2-stage ternary CFG (`bne` / `li r3, 0` / `b` / `bl searchBaseByID` / `cmpwi r3, 0`), MWCC's register allocator treats the ternary phi-node as a live range across the subsequent call and assigns `r29`.

---

## 4. `poolcheck.py` Output

```
177 pooled constants compared by VALUE across 250 paired functions
0 mismatched, 0 could not be resolved on one side
(248 pair(s) value-checked; 16 reference(s) skipped as the same named symbol on both sides; 381 float load(s) seen; 1 pair(s) skipped on length)
COVERAGE: 248 of 488 target function(s) value-checked; 240 were not checked at all (unpaired, length-mismatched, or already differing).
```
Exit code: 0 (clean).

---

## 5. Item Two: Written Landing Assessment

### Verdict: Can this unit land as written?
**NO.**

---

### 5.1. `g_padData` Assessment
- **Status in Scratch Harness**: Sized at 112 bytes (`0x70`).
- **Retail Binaries & Link Order Analysis**:
  - In `original/wiimj2d.dol`, `d_enemy_state.o` ends at `0x803142F6` with string `"dEn_c::StateID_EatOut\0"` (22 bytes).
  - The next object linked in retail is `d_enemy_toride_kokoopa.o`.
  - In retail, the FIRST `.data` symbol in `d_enemy_toride_kokoopa.o` is `__vt__18dEnTorideKokoopa_c` at `0x80314360`.
  - The distance between `0x803142F6` and `0x80314360` is 106 bytes (`0x6A`), which is the linker's 32-byte alignment padding.
- **Why `g_padData` Blocks Landing**:
  - In the current draft, the compiler emits `__vt__20KokoopaSpFumiCheck_c` (16 bytes) at the *start* of the object's `.data` section, immediately preceding `__vt__18dEnTorideKokoopa_c`.
  - `g_padData` was reduced from 128 to 112 bytes precisely to artificially absorb that 16-byte displacement in the harness.
  - At link time, if this TU were linked with `d_enemy_state.o` present and `g_padData` removed, `__vt__20KokoopaSpFumiCheck_c` would be placed at `0x80314360`, and `__vt__18dEnTorideKokoopa_c` would be displaced to `0x80314370` (+16 bytes), corrupting all state ID descriptor addresses and failing the whole binary link.
  - **Conclusion**: At link time, this TU requires **zero pad**, but it cannot land until `__vt__20KokoopaSpFumiCheck_c` is moved out of the top of `.data`.

---

### 5.2. Shadowed Headers in `scratch/gemini_round24/include/`
Every shadowed header and declaration was audited against `include/` and the retail symbol maps:

| Header | Added / Modified Declarations | Category | Rationale & Evidence |
| :--- | :--- | :---: | :--- |
| `d_a_boss_demo.hpp` | New header: `daBossDemo_c : public dActorState_c` definition and virtual methods. | **(a) Genuine** | Real actor class present in retail (`0x80064390`). Required by `dEnBoss_c` and `dActorMng_c`. |
| `d_actor_manager.hpp` | Split `mPad1[0x28]` into `mPad1[0x18]`, `daBossDemo_c *mpBossDemo` (0x18), `mPad1b[0xC]`. | **(a) Genuine** | Offset-neutral (0x18 + 4 + 0xC = 0x28). Member accessed in boss actors (`mpBossDemo->checkBattleStDemo()`). |
| `d_bc.hpp` | Added `void getWallOfs(mVec3_c *, int);` to `dBc_c`. | **(a) Genuine** | Matches retail symbol `getWallOfs__5dBc_cFP7mVec3_ci` at `0x80088910`. |
| `d_cc.hpp` | Added `void setKind(int);` to `dCc_c`. | **(a) Genuine** | Matches retail symbol `setKind__5dCc_cFi` at `0x8008D160`. |
| `d_en_fumi_check.hpp` | 1. Added `float getFumiRev();` on `FumiCcInfo_c`.<br>2. Changed `~MugenComboFumiCheck_c()` from inline `{}` to out-of-line `;`. | **(a) Genuine** | 1. Matches retail symbol `getFumiRev__12FumiCcInfo_cFv` at `0x800B07B0`.<br>2. Prevents weak inline dtor emission; retail emits `__dt__21MugenComboFumiCheck_cFv` as global at `0x800B09E0`. |
| `d_enemy_boss.hpp` | New header: `dBossLifeInf_c`, `dBossLife_Common_c`, and `dEnBoss_c : public dEn_c`. | **(a) Genuine** | Full base class hierarchy for all world bosses, with 68 virtual slots (158..225) matching retail vtable layout. |
| `d_enemy_toride_kokoopa.hpp` | New header: `dEnTorideKokoopa_c : public dEnBoss_c`. | **(a) Genuine** (with fix) | Full fortress boss class header with 89 virtual slots. Must move `KokoopaSpFumiCheck_c` declaration below `dEnTorideKokoopa_c` or to `.cpp` to avoid vtable displacement. |

**Zero Category (b) local hacks** were found. All 7 headers represent authentic structural classes and members.

---

### 5.3. New Symbols & Retail Vtable Address Inspection
Retail `.data` addresses for the three classes were checked directly against `bin/dtk/wiimj2d_symbols.txt` and `original/wiimj2d.dol`:

1. **`__vt__18dEnTorideKokoopa_c`**:
   - Retail Address: **`0x80314360`** (size `0x5E4` = 1,508 bytes, 375 slots).
   - Position: Opening `.data` symbol of `d_enemy_toride_kokoopa.cpp`.
2. **`__vt__20KokoopaSpFumiCheck_c`**:
   - Retail Address: **`0x80315298`** (size `0x10` = 16 bytes, 2 slots: dtor, `operate`).
   - Current Draft Address: Placed at `0x80314350` (before `__vt__18dEnTorideKokoopa_c`).
   - Discrepancy: In retail, this vtable sits at the **terminal end** of `d_enemy_toride_kokoopa`'s `.data` section, immediately following `__vt__33sFStateID_c<18dEnTorideKokoopa_c>` (`0x80315264`). It does **not** exist at `0x80314350`.
3. **`__vt__21MugenComboFumiCheck_c`**:
   - Retail Address: **`0x803152A8`** (size `0x10` = 16 bytes, 2 slots: dtor, `operate`).
   - Current Draft Address: Placed at `0x80314968` (inside the retail 172-byte zero gap).
   - Discrepancy: In retail, this sits at **`0x803152A8`**, immediately following `__vt__20KokoopaSpFumiCheck_c`.
4. **`__vt__12FumiCcInfo_c`**:
   - Retail Address: **`0x8031216C`** (size `0xC` = 12 bytes, weak).
   - Emitted in an earlier translation unit (`d_en_fumi_check.o` / `0x800A2870` range).

---

### 5.4. Additional Blockers to Landing

1. **Vtable Placement Order**:
   - `KokoopaSpFumiCheck_c` and `MugenComboFumiCheck_c` vtables must be placed at `0x80315298` and `0x803152A8` (after the 49 State ID tables) rather than preceding or immediately following `__vt__18dEnTorideKokoopa_c`.
2. **State ID Descriptor Placement & Alignment Gap**:
   - In retail, there is a 172-byte zero gap (`0x80314944` to `0x803149F0`) between `__vt__18dEnTorideKokoopa_c` and `@76840` (the first State ID descriptor).
   - In the draft, `@28551` begins at `0x80314970` (`0x80` bytes early), causing a `.data` alignment mismatch at link time.
3. **Unmatched Functions**:
   - Three functions remain unmatched (1,060 bytes total):
     - `setQuakeDead` (340 B target vs 352 B draft, 84 diffs)
     - `initializeState_Jump` (360 B, 6 diffs)
     - `initializeState_BigJump` (360 B, 6 diffs)
4. **Shared Header Integration**:
   - The 7 shadowed headers must be applied to `include/` and verified across all 5 binaries by Claude before this unit can be placed in `source/`.
