# CODEX_HANDOFF.md -- Codex private notebook

## Round 9: 2026-08-13

### Task
Claude''s CODEX_PROMPT.md for round 9 has two tasks:
- **Task A**: try a TU-only fix for the two EGG vector dtors (narrow idea)
- **Task B**: taxonomy of the 23 near-misses across all 8 batch reports,
  producing a ranked attack order

### Model decisions
All three sub-agents used gpt-5.6-luna (cheapest available; deepseek-v4-pro
was not available in this environment). Claude''s instructions say to use
the cheapest model.

### Orchestrator approach
Spawned three sub-agents in parallel:
- **Poincare** (gpt-5.6-luna): Task A — investigate TU-only fix for EGG vector dtors
- **Plato** (gpt-5.6-luna): Task B half — classify near-misses in BATCH1-4
- **Heisenberg** (gpt-5.6-luna): Task B half — classify near-misses in BATCH5-8

Orchestrator synthesized both halves of Task B taxonomy, combined counts,
produced ranked attack order.

### Task A result: NO
The only fix that works is the shared-header change (remove inline dtor bodies
from eggVector.h). Any TU-only approach either can''t avoid materialising an
mVec2_c reference (cvtSndObjctPos requires it) or breaks the three function
bodies. Poincare tested shadow-header variants with out-of-line destructors —
they ADD destructor calls at scope exits (incCoin +9, addRest +9,
deleteCullingYoshi +12) because MWCC can no longer prove the dtor is empty.

The existing code actually relies on MWCC seeing the inline body to elide
destructor calls. Removing the body prevents elision, making things worse.

### Task B: Combined taxonomy (10 + 13 = 23 near-misses)

**Source data:**
- Plato: BATCH1-4 → 10 functions classified
- Heisenberg: BATCH5-8 → 13 functions classified
- Orchestrator combined and verified counts

**Combined counts:**

| Class | Count |
|---|---:|
| base-register / anchor artifacts | 3 |
| register allocation only | 7 |
| instruction count differs | 7 |
| scheduling only | 1 |
| constant-folding differences | 1 |
| OTHER | 4 |

**Key finding: 12 of 23 should self-resolve at assembly** (3 anchor + 7 regalloc
+ 1 relocation-naming + 1 scheduling). That''s over half the near-misses.

**Quick confirmed fix: decRest return type.** Declared bool, target had int.
Changing to int produced exact match. Four instructions saved. Another CFront
return-type casualty.

**Biggest payoff targets:**
1. createCourseInit (345 vs 352) — 7 instrs missing, known lever (hoist mVec3_c
   to function scope), getFileP inline budget gauge
2. incCoin (134 vs 130) — 4 extra instrs from separate array materialisation
3. checkCorrectCreateInfo (105 vs 105) — same count, constant-folding diffs,
   may resolve at assembly

### Files written this round
- scratch/codex_round9/taskA_narrow_fix.md (Poincare)
- scratch/codex_round9/taskB_batches1to4.md (Plato)
- scratch/codex_round9/taskB_batches5to8.md (Heisenberg)
- CODEX_RESPONSE.md (overwritten)
- CODEX_HANDOFF.md (this file, overwritten)