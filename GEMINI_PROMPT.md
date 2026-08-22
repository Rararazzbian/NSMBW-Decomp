# Work order — round 24 (continued)

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Your round 24 was cut off by a usage limit. The work survived; the report did not.

You never wrote `GEMINI_RESPONSE.md` — it still contains your round 23. But your
object file and source in `scratch/gemini_round24/` are intact, so **I scored
them myself.** You do not need to redo any of it:

    GAINED: 2 functions, 628 bytes
    LOST:   0 functions,   0 bytes

    + 376  shellAtkEffect
    + 252  executeState_AttackEnd

That puts you at **231/251 under the gate with the naming-artifact rule
applied** (229 raw plus the two known artifacts). Both closures are banked and
verified. `shellAtkEffect` went from 52 diffs to zero, which was the largest
content problem on the list — good round, and it counted even though it was
interrupted.

**Continue in `scratch/gemini_round24/`. Do not start a fresh directory and do
not re-derive anything** — your current source is the baseline for this round.

---

## Work so a truncated round still banks value

You have now lost a report to a usage limit once. Two process changes, both
cheap:

1. **Write your source to disk after every single closure**, not at the end of
   the round. That is the only reason this round counted at all — your `.o` and
   `.cpp` were on disk at 21:57 even though nothing else was.
2. **Keep a running `GEMINI_RESPONSE.md` and append to it as you go**, rather
   than composing it at the end. A half-written report of real closures beats a
   perfect report you never reach. If you are cut off again, whatever is on disk
   is what I score.

Order the work below strictly. If you get through two items, that is a round.

---

## Round 24 continued — order of work

**20 functions genuinely unmatched, 3,940 bytes.** Ignore `__sinit` (4 diffs)
and `executeState_ShellAtk_St` (2 diffs) — both are pure `SYM<n>` renumbering
and count as matched under the naming-artifact rule. Do not touch them.

1. **`executeState_AttackSearch` — 512 B, 512/512, TWO diffs.** Still open, and
   still the best value on the board. Your `li r3, 0` / `li r4, 0` reading was
   right; finish it. This is the one item I would not want to lose to another
   interruption, so do it first.

2. **`initializeState_Jump` / `initializeState_BigJump` — 360 B each, 6 diffs.**
   Both are the correct length with correct control flow. **First state which
   register file the diffs are in.** If `f0`..`f13`, they are volatile, the
   declaration-order lever does not apply, `AGENT_CONTEXT.md` records that as a
   measured bounded negative — say so and move on without grinding. If
   `f14`..`f31`, they are callee-saved and the lever *does* apply; that boundary
   is in the context file and it is worth 720 bytes. They are also mirrors, and
   a mirror does not necessarily take the mirrored fix — measure both.

3. **The length-wrong group, biggest first.** These are content problems, not
   codegen problems — the lengths are wrong, so work is missing or extra, and no
   register lever will help until the length is right:

       340 B  setQuakeDead      (352/340, 84 diffs)
       268 B  preExecute        (252/268, 48 diffs)
       208 B  moveRevise        (204/208, 22 diffs)
       204 B  calcAttackTarget  (196/204, 21 diffs)
       188 B  calcJumpRate      (176/188, 45 diffs)
       188 B  movelimitCheck    (196/188, 39 diffs)
       180 B  checkDownJump     (160/180, 42 diffs)

   `shellAtkEffect` was in this group at 52 diffs and you closed it outright, so
   the approach you used there is the one to repeat. Whatever you did, do it
   again on `setQuakeDead` and `preExecute`.

Do not touch `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`,
`configure.py`, `QWEN_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build destroys that.

---

## Reporting

Round 23's format was right; keep it.

- Baseline is **231/251**, the figure above — do not recompute it from scratch,
  and note that the two closures already banked are *not* new gains this round.
- **GAINED and LOST by name.** Round 23 was your first clean loss report and the
  interrupted round 24 was clean too. Two in a row.
- **Count the rows of your own GAINED table against your heading** before
  writing the summary. That was the only defect left in round 23.
- Per function: draft size first, then target size, then status.
- For Jump/BigJump: the register file, explicitly, before any conclusion.
- `poolcheck.py` output.
