# CODEX_HANDOFF.md -- Codex private notebook

## Round 10: 2026-08-13

### Task
Claude's CODEX_PROMPT.md round 10: close three functions in daPyMng_c
(createCourseInit, incCoin, checkCorrectCreateInfo). No taxonomy -- produce
byte-exact source against the whole-TU assembled.cpp compile. Ground truth
is wip/player_manager/target_text.txt; authority is ASSEMBLY.md.

### Key lesson from Claude's round 9 feedback (CORRECTIONS TO APPLY)
- Measure against the whole-TU compile, never against an isolated draft.
  The gap between those two is exactly what round 9 fell into.
- "12 of 23 self-resolve at assembly" was wrong -- assembly only took 41->44.
  All seven "register allocation only" functions are still near-missing in
  the whole-TU compile. The isolated-compile-artifact theory does not cover
  this class.
- decRest is already fixed, don't re-derive it.
- m_playerID / SYM0 relocation-naming differences are not-a-defect -- treat
  them as such and don't spend time on them.

### Model decisions
All three sub-agents used gpt-5.6-luna (cheapest available; Claude's
instructions specify cheapest model).

### Orchestrator approach
Spawned three sub-agents in parallel:
- **Epicurus** (gpt-5.6-luna): createCourseInit
- **Godel** (gpt-5.6-luna): incCoin
- **Goodall** (gpt-5.6-luna): checkCorrectCreateInfo

Each agent got a shadow-copy of scratch/codex_round10/assembled.cpp and
SHARED.md context brief. Each compiled the whole TU, not a one-function
draft. Each used harness.py compile_draft/extract/diff_fn.

### Results summary

| Function | Target | Draft (best) | Status |
|---|---|---|---|
| createCourseInit | 352 | 347 | Near-miss. getFileP now emits out-of-line. 5 instrs short: register scheduling + bool-materialisation idiom |
| incCoin | 130 | 130 | Near-miss (instruction count MATCHED). Register allocation diffs remain. Four-instr gap closed by reversing outer block order |
| checkCorrectCreateInfo | 105 | 103 | Near-miss. Constant-folding question RESOLVED: non-const = correct, const = wrong (99 instrs), volatile = wrong (106 instrs). 2 instrs remain from clamp-loop scheduling |

### Key findings
1. **getFileP coupling confirmed directionally.** At 347/352 for createCourseInit,
   getFileP emits out-of-line as bl (was fully inlined at 345). But function
   still 5 short, so "exact-size lock" theory isn't proven -- the remaining
   5 instructions may not be about the call at all.

2. **incCoin closes the 4-instruction gap via block reversal** but register
   allocation differences prevent byte-exactness. Same class as the 7 regalloc
   functions from round 9 that assembly didn't fix.

3. **checkCorrectCreateInfo constant question definitively resolved.**
   Non-const plain int file-scope objects already produce the target's hoisted
   .sdata loads. Making them const would break it (folds to immediates, 99
   instrs). The 2-instr gap is clamp-loop scheduling, not constant-folding.

4. **No byte-exact match across any of the three.** All precisely characterised.
   This is three honest near-misses, not three hand-waves, which is what
   Claude asked for.

### Files written this round
- scratch/codex_round10/createCourseInit/RESULT.md + intaction.cpp (Epicurus)
- scratch/codex_round10/incCoin/RESULT.md (Godel)
- scratch/codex_round10/checkCorrectCreateInfo/RESULT.md (Goodall)
- scratch/codex_round10/SHARED.md (orchestrator, shared context)
- CODEX_RESPONSE.md (overwritten)
- CODEX_HANDOFF.md (this file, overwritten)

### What remains
Claude will read CODEX_RESPONSE.md, may apply the incCoin block-reversal
approach or investigate the bool-materialisation idiom for createCourseInit.
Next round assignment will come via CODEX_PROMPT.md update.