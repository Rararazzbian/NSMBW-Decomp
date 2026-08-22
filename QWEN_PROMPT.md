# Work order — round 27

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 26 produced no object file. Nothing was compiled.

I checked before reading your report:

    scratch/round26/d_bg_ctr/d_bg_ctr.cpp      byte-identical to round 25
    scratch/round26/d_bg_ctr/draft_disasm.txt  byte-identical to round 25
    scratch/round26/d_bg_ctr/*.o               does not exist

The `MATCHED: 32  DIFFER: 7` you reported is round 25's result, re-run on round
25's unchanged output. Every function in your table says *"Compiled: no new
variant; baseline only."* That is accurate, and it is the whole problem.

**I authorised "analyse and stop" for `fn_80080900` alone**, as a timebox on a
function that had already burned two rounds. You applied it to all seven —
including `calc`, which the brief called highest-confidence and told you to do
first.

`AGENT_CONTEXT.md` has carried the rule since round 23 and it has not changed:
**prose that has not compiled is not a measurement.** A body that compiles at
200 diffs is worth more than one that reads correctly and has never been built,
because the first can be iterated and the second cannot. Seven proposals at
"confidence: high" are worth one compile.

**You did not fabricate anything, and that matters.** You reported GAINED none,
LOST none, and labelled every function uncompiled rather than letting the
unchanged 32/7/0 read as progress. Keep that exactly as it is.

---

## Your diagnoses are good. Three of them are better than any previous round.

This is not a wasted round, it is an unexecuted one. Specifically:

**`fn_80080E40` — you found it, and I verified it is real.** You identified the
register file as GPR-only (asked for, and correct), then found a concrete
draft-only gate:

    scratch/round26/d_bg_ctr/d_bg_ctr.cpp:560
    if (!mEntryFlag || mpActor == nullptr) {

The target has no such gate — it goes straight to the `0xDC` byte test after the
prologue. That is a real defect, plausibly the whole +3, and **it is a one-line
deletion you have already located.** The previous round called this function
"not source-addressable" and stopped. You did better and then also stopped.

**`calc` — you reached the right conclusion.** Callee-saved FPRs, `f30/f31`
against `f28/f29`, and you listed the correct levers in the correct order. That
is the analysis done. Now run it.

**`fn_80080900` — the liveness table is exactly what I asked for** and it is the
most useful artifact anyone has produced on this function. Eleven register roles
and four stack objects, with lifetimes. Build from it.

---

## Round 27 — one rule

**Every item below must end in a compiled object and a diff count.** For each
function report the number. `"Compiled: no"` is not an acceptable value in this
round's table — if you run out of time, report fewer functions, not more
proposals.

Order, and all of it is work you have already designed:

1. **`fn_80080E40` (+3)** — delete the `mEntryFlag`/`mpActor` gate at line 560,
   express the target's first predicate in target order. Compile. One line.
2. **`calc` (+4)** — run your own list, in your own order: declaration order of
   the float locals; then named temporaries in target READ order; then split
   declaration from assignment; then compose evaluation order with the read-side
   def-point. **Compile each variant and report its word count**, including the
   ones that get worse. Four numbers is a good result here even if none is zero.
3. **`revisePos` (72/72)** — the read-order lever, third round on the list, one
   compile. Target reads `0x9C`, `0xB4`, `0xB0`, `0x98`, `0xAC`, `0x94` — you
   have the order written down already.
4. **`fn_8007FFA0` (−8)** and **`addDokanMoveDiff` (−7)** — missing content, per
   your own read. Write the missing stores and lifetimes and compile.
5. **`fn_80080900` (−48)** — one pass building the objects your liveness table
   describes: an explicit four-corner array, edge start/end, and a hit-result
   vector, with lifetimes spanning both geometry calls. Compile it whatever the
   frame comes out as. If it does not converge, that is fine — but I want the
   frame size and `_savegpr` level of the attempt, not another proposal.

`fn_80080670` (−3) last, only if time remains.

Work in `scratch/round27/`. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `GEMINI_*`, `CODEX_HANDOFF.md`, or
`HANDOFF.md`. **Do not run `ninja`, `configure.py`, `progress.py` or `land.py`**
— the tree is green, all five binaries byte-exact, and a concurrent build
destroys that.

---

## Reporting

Per function: target length, **draft length from the object you built this
round**, `_savegpr` level, frame size, and diff count.

**GAINED and LOST by name.** Four rounds accurate; keep it.

`poolcheck.py` on the final object.

Drop the "Offset-perturbing" field — it is not something I asked for and it did
not carry information. Replace it with the variant list: for each function, what
you compiled and what number came back.
