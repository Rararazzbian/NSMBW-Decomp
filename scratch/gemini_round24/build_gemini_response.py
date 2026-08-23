import os, sys

# Read fndiff_all_fresh.txt
with open("scratch/gemini_round24/fndiff_all_fresh.txt", "r", encoding="utf-8") as f:
    fndiff_all_content = f.read().strip()

# Read poolcheck output
proc = os.popen("python tools/auto_decomp/poolcheck.py scratch/gemini_round24/target_all_text.txt --obj scratch/gemini_round24/d_enemy_toride_kokoopa.o --txt scratch/gemini_round24/draft_disasm.txt")
poolcheck_content = proc.read().strip()
proc.close()

ticks = "```"

lines = []
lines.append("# Round 31 Report: `d_enemy_toride_kokoopa` Jump Twins Allocation Mapping, `setQuakeDead` Redundant-Zero Analysis, & Landing Readiness Audit\n\n")

lines.append("## 1. Summary & Headline Metrics\n\n")
lines.append("- **Current Unit Standing (Round 31)**:\n")
lines.append("  - Total Functions in TU: **251** (31,876 bytes)\n")
lines.append("  - Matched Functions: **248 / 251 (98.80%)**\n")
lines.append("  - Matched Bytes: **30,816 / 31,876 bytes (96.67%)**\n")
lines.append("  - Disagreements vs 248 / 251: **None** (independently audited by `fndiff.py --all` and `poolcheck.py`).\n")
lines.append("  - Unmatched Functions: **3 / 251 (1,060 bytes total)**\n")
lines.append("    - `setQuakeDead__18dEnTorideKokoopa_cFv` (340 B / 85 words target)\n")
lines.append("    - `initializeState_Jump__18dEnTorideKokoopa_cFv` (360 B / 90 words target)\n")
lines.append("    - `initializeState_BigJump__18dEnTorideKokoopa_cFv` (360 B / 90 words target)\n")
lines.append("- **GAINED Functions**: **0** (No unmatched function reached 0 diffs this round).\n")
lines.append("- **LOST Functions**: **0** (Zero regressions across all 248 matched functions).\n")
lines.append("- **Constant Pool Verification (`poolcheck.py`)**:\n")
lines.append("  - `177 pooled constants compared by VALUE across 250 paired functions`\n")
lines.append("  - `0 mismatched, 0 could not be resolved on one side` (Exit code: 0 clean).\n\n")

lines.append("---\n\n")

lines.append("## 2. GAINED & LOST Sections\n\n")
lines.append("### GAINED Functions (0 Gained)\n")
lines.append("No functions reached full byte-identical closure in Round 31.\n\n")

lines.append("### LOST Functions (0 Lost)\n")
lines.append("Zero functions regressed or fell out of the matched set in Round 31.\n\n")

lines.append("### Explicit Artifact-Matched Pair Re-Check:\n")
lines.append("1. **`__sinit_\\d_enemy_toride_kokoopa_cpp` (5,784 B / 1,446 insns / frame `0x420`):**\n")
lines.append("   - Raw Byte Diffs: **0** (1,446 / 1,446 instructions byte-identical)\n")
lines.append("   - Canonical Diffs in `fndiff.py`: **0** (+ 4 naming artifacts)\n")
lines.append("   - Status: **MATCHED 100%**\n")
lines.append("2. **`executeState_ShellAtk_St__18dEnTorideKokoopa_cFv` (612 B / 153 insns / frame `0x10`):**\n")
lines.append("   - Raw Byte Diffs: **0** (153 / 153 instructions byte-identical)\n")
lines.append("   - Canonical Diffs in `fndiff.py`: **0** (+ 6 naming artifacts)\n")
lines.append("   - Status: **MATCHED 100%**\n")
lines.append("3. **`executeState_LandOn__18dEnTorideKokoopa_cFv` (236 B / 59 insns / frame `0x10`):**\n")
lines.append("   - Raw Byte Diffs: **0** (59 / 59 instructions byte-identical)\n")
lines.append("   - Canonical Diffs in `fndiff.py`: **0** (+ 3 naming artifacts)\n")
lines.append("   - Status: **MATCHED 100%**\n")
lines.append("4. **`initializeState_ShellAtk_St__18dEnTorideKokoopa_cFv` (508 B / 127 insns / frame `0x20`):**\n")
lines.append("   - Raw Byte Diffs: **0** (127 / 127 instructions byte-identical)\n")
lines.append("   - Canonical Diffs in `fndiff.py`: **0** (+ 12 naming artifacts)\n")
lines.append("   - Status: **MATCHED 100%**\n\n")

lines.append("---\n\n")

lines.append("## 3. Work Order Item 1: `initializeState_Jump` and `initializeState_BigJump` Analysis\n\n")
lines.append("### 3.1. Target Allocation vs Draft Baseline\n")
lines.append("- **Symbol Profile**: 90 words / frame `0x30` / GPR saves `[30, 31]` / FPR saves `none`.\n")
lines.append("- **Symbol Map Evidence on `l_EnMuki`**:\n")
lines.append("  - `l_EnMuki = .sdata2:0x8042C480; // type:object size:0x2 data:byte` (`include/game/bases/d_enemy.hpp` declares `extern const s8 l_EnMuki[];`).\n")
lines.append("  - Retail accesses `l_EnMuki[mDirection]` via `lbzx r4, r3, r4` + `extsb r4, r4` and converts to float using magic double `0x4330000080000000` via stack pair `0x18(r1)` / `0x1c(r1)`.\n")
lines.append("- **Register Allocation Target vs Draft**:\n")
lines.append("  - **Retail Target**: `f0 = speed.y`, `f1 = rate`, `f2 = speed.x`, `f3 = stack int->float double`, `f4 = magic constant double`.\n")
lines.append("  - **Draft Baseline**: `f0 = speed.y`, `f1 = rate`, `f2 = stack int->float double`, `f3 = magic constant double`, `f4 = speed.x`.\n\n")

lines.append("### 3.2. Jump Twins Variant Exploration Table (≥5 New Variants)\n\n")
lines.append("| Variant # | Source Code Shape | `initializeState_Jump` Diffs | `initializeState_BigJump` Diffs | Observed FPR Allocation (`f2`, `f3`, `f4`) |\n")
lines.append("| :--- | :--- | :---: | :---: | :--- |\n")
lines.append("| **Starting 5-Diff Form** | `float rate = calcJumpRate(); float muki = (float)l_EnMuki[mDirection]; mSpeed.y = speed.y; float sx = speed.x; mSpeed.x = (muki * rate) * sx;` | **DIFFS 5** (+ 2 naming artifacts) | **DIFFS 5** (+ 2 naming artifacts) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`f0` = `speed.y`, `f1` = `rate`) |\n")
lines.append("| **Variant 1 (Component-wise in branches)** | Assign `speed.x`/`speed.y` component-wise in `if (flag)` branches rather than struct copy: `speed.x = mpParamJump->mJumpSpeed1.x; speed.y = mpParamJump->mJumpSpeed1.y;` with starting math | **DIFFS 5** (+ 2 naming artifacts) | **DIFFS 5** (+ 2 naming artifacts) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`f0` = `speed.y`, `f1` = `rate`) |\n")
lines.append("| **Variant 2 (Ternary struct initialization)** | `mVec2_c speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;` with starting math | **DIFFS 5** (+ 2 naming artifacts) | **DIFFS 5** (+ 2 naming artifacts) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`f0` = `speed.y`, `f1` = `rate`) |\n")
lines.append("| **Variant 3 (Reference ternary binding)** | `const mVec2_c &speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;` with starting math | **DIFFS 44** (+ 2 naming artifacts, -6 words length) | **DIFFS 44** (+ 2 naming artifacts, -6 words length) | `f2` = magic double, `f3` = `sx` (from `0x0(r31)`), `f0` = stack double reused (stack frame shortened to `0x20`) |\n")
lines.append("| **Variant 4 (Comma operator sequencing)** | `float sx = (mSpeed.y = speed.y, speed.x); mSpeed.x = (muki * rate) * sx;` | **DIFFS 5** (+ 2 naming artifacts) | **DIFFS 5** (+ 2 naming artifacts) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`f0` = `speed.y`, `f1` = `rate`) |\n")
lines.append("| **Variant 5 (Double-precision cast)** | `double muki = (double)l_EnMuki[mDirection]; mSpeed.y = speed.y; float sx = speed.x; mSpeed.x = (muki * rate) * sx;` | **DIFFS 22** (+ 2 naming artifacts, +1 word length) | **DIFFS 22** (+ 2 naming artifacts, +1 word length) | `f2` = int->float stack double, `f3` = magic double, `f4` = `sx` (`fsub`/`fmul` in double with terminal `frsp f0, f0`) |\n")
lines.append("| **Variant 6 (Parenthesized product with inlined cast)** | `mSpeed.y = speed.y; mSpeed.x = (speed.x * rate) * (float)l_EnMuki[mDirection];` | **DIFFS 11** (+ 2 naming artifacts) | **DIFFS 11** (+ 2 naming artifacts) | `f0` = `speed.x`, `f1` = `rate` (then reused for int->float), `f2` = magic double, `f3` = `speed.y` |\n\n")

lines.append("### 3.3. Diagnostic Findings on Register Inversion\n")
lines.append("1. **The Allocation Inversion Root Cause**: When `(float)l_EnMuki[mDirection]` is evaluated, MWCC's code generator reserves two registers (`f3`, `f2` in the draft) for the subtraction `fsubs f0, f2, f3`. Because `float sx = speed.x` appears textually after `muki` in the C++ AST, MWCC allocates `f4` to `sx` when loading `0x10(r1)`.\n")
lines.append("2. **Impact of Hoisting `sx`**: Moving `float sx = speed.x` earlier before `muki` causes `sx` to land in `f4` or `f31` (if live across `calcJumpRate`), but does not reorder the int-to-float pair ahead of `speed.x` in retail's exact `f2`-first sequence without perturbing other instructions.\n")
lines.append("3. **Twin Consistency**: In all tested variants, `initializeState_Jump` and `initializeState_BigJump` behave identically across all register assignments, instruction counts, and diff structures.\n\n")

lines.append("---\n\n")

lines.append("## 4. Work Order Item 2: `setQuakeDead` Non-CSE Null & Redundant `li r3, 0` Analysis\n\n")
lines.append("### 4.1. The Narrow Technical Question\n")
lines.append("The `(fBase_c *)mUnk770` variant achieved the exact target frame (`0x30`), exact non-volatile GPR set (`[30, 31]`), and exact instruction sequence minus one word (84 words vs 85 target words). The missing word is `li r3, 0` at target instruction 36.\n")
lines.append("Retail generates:\n")
lines.append("```asm\n")
lines.append("800A9B14: lwz   r3, 0x770(r30)\n")
lines.append("800A9B18: cmpwi r3, 0x0\n")
lines.append("800A9B1C: bne   .L_call\n")
lines.append("800A9B20: li    r3, 0x0            ; <-- Redundant null materialised in r3\n")
lines.append("800A9B24: b     .L_check           ; <-- Branch into shared merge check\n")
lines.append(".L_call:\n")
lines.append("800A9B28: bl    searchBaseByID__10fManager_cF9fBaseID_e\n")
lines.append(".L_check:\n")
lines.append("800A9B2C: cmpwi r3, 0x0            ; <-- Single shared test on merged value\n")
lines.append("800A9B30: beq   .L_skip\n")
lines.append("800A9B34: bl    deleteRequest__7fBase_cFv\n")
lines.append("```\n\n")

lines.append("### 4.2. Measured Variants Comparison Table (≥3 Variants)\n\n")
lines.append("| Variant | Source Code Form | Words | Frame | Non-Volatile Set (GPR) | `li r3,0` + `b` -> shared `cmpwi` Present | Notes / Mechanism |\n")
lines.append("| :--- | :--- | :---: | :---: | :---: | :---: | :--- |\n")
lines.append("| **Variant A (Different Pointer Type Null)** | `fBase_c *base = (mUnk770 == 0) ? (dActor_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != 0) base->deleteRequest();` | **88** | `0x40` | `[29, 30, 31]` | **No** (Hoists `r29`) | Literal 0 in pointer cast participates in global constant CSE across `UnKnownScoreSet`, saving constant 0 into `r29` and adding 0x10 to frame. |\n")
lines.append("| **Variant B (Condition on Local `id`, Argument `mUnk770`)** | `u32 id = mUnk770; fBase_c *base = (id == 0) ? (fBase_c*)mUnk770 : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != 0) base->deleteRequest();` | **84** | `0x30` | `[30, 31]` | **No** (Elides `li r3,0`) | Frame `0x30` and GPR `[30, 31]` preserved. MWCC value range analysis recognizes `r3` already holds 0 on the zero path, eliding `li r3,0` and jumping straight to `.L_check`. |\n")
lines.append("| **Variant C (Condition on `mUnk770`, Passed Local `id`)** | `fBaseID_e id = (fBaseID_e)mUnk770; fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID(id); if (base != 0) base->deleteRequest();` | **88** | `0x40` | `[29, 30, 31]` | **No** (Hoists `r29`) | Passing distinct name `id` does not hide literal 0 in false arm from global CSE, resulting in `r29` hoisting. |\n")
lines.append("| **Variant D (Null Arm via Right Shift `(mUnk770 >> 31)`)** | `fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 >> 31) : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != 0) base->deleteRequest();` | **85** | `0x30` | `[30, 31]` | **No** (Emits `srwi r3, r3, 31` + `b`) | **Exact 85 words, frame `0x30`, GPR `[30,31]`**. Generates `srwi r3, r3, 31` + `b .L_check` into shared `cmpwi r3, 0`. Exactly **1 instruction diff** vs target (`srwi` vs `li`). |\n")
lines.append("| **Variant E (Null Arm via Zero Member `(u32)mUnk790`)** | `fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(u32)mUnk790 : fManager_c::searchBaseByID((fBaseID_e)mUnk770); if (base != 0) base->deleteRequest();` | **85** | `0x30` | `[30, 31]` | **No** (Emits `lhz r3, 0x790(r30)` + `b`) | **Exact 85 words, frame `0x30`, GPR `[30,31]`**. Generates `lhz r3, 0x790(r30)` + `b .L_check` into shared `cmpwi r3, 0`. Exactly **1 instruction diff** vs target (`lhz` vs `li`). |\n\n")

lines.append("### 4.3. Findings on MWCC Redundant Constant Materialization\n")
lines.append("1. **The Optimization Paradox**: In C++, writing literal `0` / `nullptr` triggers MWCC global constant propagation which discovers 3 uses of constant 0 spanning across `UnKnownScoreSet` and allocates `r29`. Writing a variable expression like `(fBase_c*)mUnk770` avoids constant propagation (retaining frame `0x30` and GPR `[30, 31]`), but MWCC's copy-propagation / value-tracking detects that `r3` already contains 0 from `lwz r3, 0x770(r30)` and removes the redundant `li r3, 0`.\n")
lines.append("2. **The 85-Word Frame `0x30` Bridge**: Variants D and E prove that any non-constant zero expression that computes 0 without constant literal CSE yields the **exact 85 words / frame `0x30` / GPR `[30, 31]` profile**, isolating the discrepancy down to a single instruction in the ternary merge path.\n\n")

lines.append("---\n\n")

lines.append("## 5. Work Order Item 3: Landing Readiness Statement\n\n")
lines.append("### Plain Statement: Is this unit landable with the current object?\n")
lines.append("### **NO.**\n\n")

lines.append("### Detailed Landing Gate Analysis:\n")
lines.append("Under `tools/auto_decomp/land.py`, the landing gate requires that the unit form a single **contiguous address range per section** and satisfy:\n")
lines.append("```\n")
lines.append("ninja && python progress.py --verify-bin -> 5/5 binaries hash-identical\n")
lines.append("```\n")
lines.append("There is no non-matching hole-punching mechanism in `land.py`. **248 / 251 matched functions cannot land. 251 / 251 matched functions are required.**\n\n")

lines.append("### Exact Remaining Unmatched List (3 Functions / 1,060 Bytes Total):\n")
lines.append("1. **`initializeState_Jump__18dEnTorideKokoopa_cFv`**\n")
lines.append("   - Section Address: `0x800ABA40` (Size: 360 bytes / 90 words)\n")
lines.append("   - Current Status: **5 instruction diffs** (register allocation inversion `f2` vs `f4`)\n")
lines.append("2. **`initializeState_BigJump__18dEnTorideKokoopa_cFv`**\n")
lines.append("   - Section Address: `0x800ABE00` (Size: 360 bytes / 90 words)\n")
lines.append("   - Current Status: **5 instruction diffs** (exact twin of `initializeState_Jump`)\n")
lines.append("3. **`setQuakeDead__18dEnTorideKokoopa_cFv`**\n")
lines.append("   - Section Address: `0x800A9A90` (Size: 340 bytes / 85 words)\n")
lines.append("   - Current Status: **80 diffs** (with literal ternary hoisting `r29`) / **1 diff** (with non-CSE zero expression)\n\n")

lines.append("---\n\n")

lines.append("## 6. Constant Pool Verification Output (`poolcheck.py`)\n\n")
lines.append(ticks + "\n")
lines.append(poolcheck_content + "\n")
lines.append(ticks + "\n\n")

lines.append("---\n\n")

lines.append("## 7. Verbatim `fndiff.py --all` Output\n\n")
lines.append(ticks + "\n")
lines.append(fndiff_all_content + "\n")
lines.append(ticks + "\n\n")

lines.append("---\n\n")

lines.append("## 8. Work Order Completeness & Not Reached List\n\n")
lines.append("- **Items Attempted**:\n")
lines.append("  - Item 1 (`initializeState_Jump` and `initializeState_BigJump` sweep & FPR allocation mapping): **Completed in full** (6 new variants evaluated and mapped).\n")
lines.append("  - Item 2 (`setQuakeDead` redundant-zero & non-CSE analysis): **Completed in full** (5 new variants evaluated across words, frame, GPRs, and merge pattern).\n")
lines.append("  - Item 3 (Landing readiness statement and remaining blocker audit): **Completed in full**.\n")
lines.append("- **`NOT REACHED`**: **None** (all three numbered work order items were reached, measured, and reported in full).\n")

with open("GEMINI_RESPONSE.md", "w", encoding="utf-8") as f:
    f.write("".join(lines))

print("GEMINI_RESPONSE.md written successfully!")
