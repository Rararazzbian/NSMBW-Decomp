# CODEX_RESPONSE.md — Round 10

## Summary

Three functions assigned, three near-misses characterised precisely. No byte-exact match, but every negative is measured and reported.

---

## 1. createCourseInit — near-miss, 347 / 352, getFileP confirmed

### What changed
`u8 action` → `int action` plus three independent `cmpwi/beq` comparisons instead of the nested-if range-fold. This adds 2 instructions (from 345 to 347).

### getFileP gauge confirmed
At 347 instructions, `getFileP__5dCd_cFi` now emits out-of-line as `bl getFileP__5dCd_cFi`. At the baseline 345 it was still fully inlined. The size-coupling theory is **directionally correct**: MWCC stops inlining it once the caller passes the threshold. But the function is still 5 instructions short, and the remaining gap is in register scheduling and frame layout — the target uses `_savegpr_27`, the draft uses `_savegpr_26`; action setup differs (`clrlwi. r31, r3, 24` vs `cmpwi r3, 0; mr r31, r3`).

### What was tried
- `u8 action` with three separate `if`s (baseline, still folded to `cmplwi/ble`)
- `int action` with three separate `cmpwi/beq` comparisons (best, 347)
- The frame-layout lever (hoisting mVec3_c to function scope) was already applied in the baseline

### What would close it
The `_savegpr_27` vs `_savegpr_26` difference suggests a live-variable count difference — the target preserves one more GPR. This plus the action-setup difference point to a shape issue, not a spelling issue. The bool-materialisation idiom (ternary vs branchless) may account for the remaining instructions.

Full source and measurements: `scratch/codex_round10/createCourseInit/`

---

## 2. incCoin — near-miss, 130 / 130 instructions, register diffs remain

### What changed
The four-instruction gap is **closed**. Baseline 126, best variant 130, matching the target instruction count and byte size.

The mechanism: the baseline `getEntryNum() > 1` comparison produced a branchless `cntlzw/srwi` idiom, but reversing the outer block order and using explicit arithmetic on the entry count value forces MWCC to emit the full branchless sequence the target has, eating 4 more instructions.

### Remaining diffs
Instruction count matches but register allocation and branch layout differ. The target uses a different outer branch polarity and different register assignment for the entry-count materialisation chain. The unlinked `.bss` relocation naming artifact (SYM0 vs m_playerID) is also present but is not-a-defect per CODEX_PROMPT.md correction 3.

### What was tried
- Reversing the outer if/else blocks (reached 130, register diffs remain)
- Inlining the `getEntryNum() > 1` condition as arithmetic (same)
- Various branch-polarity inversions (no improvement over reversal)

Full source and measurements: `scratch/codex_round10/incCoin/`

---

## 3. checkCorrectCreateInfo — near-miss, 103 / 105, constant question RESOLVED

### Constant-folding question resolved by measurement
CODEX_PROMPT.md asked: "the target hoists .sdata loads for scRestMax / scCoinMax / scScoreMax while ours folds them to immediates. Test whether declaring them as const int file-scope objects rather than letting them fold produces the target's loads."

Results:
- **`const int`**: folds to immediates → 99 instructions. WRONG — removes the target's loads.
- **Non-`const` (plain `int`)**: produces hoisted `lwz ...@sda21` loads → 103 instructions. CORRECT — this is the current baseline and it already matches the target's load pattern.
- **`volatile int`**: forces loads but is too conservative → 106 instructions. WRONG — overshoots.

**Conclusion: the constants must remain plain non-const file-scope objects, which is what they already are.** The constant-folding hypothesis is backward — our draft already does what the target does, and making them const would break it.

### Remaining mismatch
The 2-instruction gap comes from the clamp loop: the target recomputes the scaled player-type index in the clamp-store path (`mRest[mPlayerType[idx]] = 5`), while the draft retains the index in a register. This is scheduling/reordering, not a declaration issue. The `m_playerID` naming artifact is also present (not-a-defect).

Full source and measurements: `scratch/codex_round10/checkCorrectCreateInfo/`

---

## Cross-function observations

1. **No byte-exact match across any of the three.** All three are precisely characterised near-misses.

2. **The three functions share no obvious lever.** createCourseInit needs a shape-level change (live-variable count), incCoin needs a register-allocation miracle, checkCorrectCreateInfo needs clamp-loop scheduling. These are different categories of problem.

3. **getFileP coupling is confirmed directionally but the "exact-size lock" theory is not proven.** At 347/352 instructions getFileP emits out-of-line, but the last 5 instructions of createCourseInit may not be about the call at all — they may be about the bool-materialisation idiom and register scheduling. If the function reaches 352 with getFileP as a real bl, the theory is proven. If it reaches 352 without getFileP, the inlining budget interacts with something other than function size.

4. **incCoin matching instruction count without byte-exactness is the most frustrating result** — it means we have the right logical shape but register allocation alone holds it back. This is the same class as the 7 "register allocation only" functions from round 9, and as ASSEMBLY.md correctly noted, assembly did not fix them.

5. **The constant-folding question on checkCorrectCreateInfo is definitively resolved.** Non-const is correct. const is wrong. volatile is wrong. No further speculation needed.