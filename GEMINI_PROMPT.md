# Work order — round 29

**Read `AGENT_CONTEXT.md` first**, including the two new sections at the end:
*"Count diffs on CANONICALISED text"* and *"Register ALLOCATION order and load
EMISSION order are separate levers"*. The first one changes how you should be
scoring `setQuakeDead`.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 28: the vtable diagnosis was right and the fix landed

Blockers #1 and #2 cleared in one round, from a defect you found yourself and
had merely mis-labelled as a constraint. `g_padData` to zero, the `0x80`
state-ID displacement gone with it, and `__sinit` re-checked explicitly rather
than assumed — after round 25, that check was the part I most wanted to see and
you did it without being reminded twice.

Zero LOST, and you named the artifact-matched pair. The protocol is holding.

**Where the unit stands: 248/251, 30,816 bytes, two of four blockers cleared.**
The remaining two are `setQuakeDead` and the header promotion, and the header
promotion is mine.

---

## Your diff counts are inflated, probably by a lot

I closed `revisePos` in the other unit this morning. It was reported at 16
differing instructions. **Ten of the sixteen were names**, not code:

    T: b .L_800801F8                D: b .L_00000D38          (6 branch labels)
    T: bl fn_8007FFA0               D: bl fn_8007FFA0__FP...  (3 mangled calls)
    T: lfs f0, "@69447_8042C168"    D: lfs f0, "@13813"       (1 pooled float)

The target names labels after retail addresses, the draft after object offsets;
static helpers carry a mangling suffix on one side; pooled constants are
numbered per object. The real count was **6**, and at 6 the function was
readable end to end and closed within the hour.

**`setQuakeDead` at "84 diffs" and "50 diffs" has never been measured this way.**
A 340-word function with branches and float constants could easily be carrying
twenty-plus naming rows. Your first job is to find out what the number actually
is.

Use `tools/auto_decomp/fndiff.py` — new, committed, and it also prints words /
frame / GPR saves / FPR saves for both sides:

    python tools/auto_decomp/fndiff.py TARGET.txt DRAFT.txt setQuakeDead__... -v
    python tools/auto_decomp/fndiff.py TARGET.txt DRAFT.txt --all

---

## Completeness

All four items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Round 29 — order of work

### 1. Re-baseline with `fndiff.py --all`

One run over your current object. It gives me an independent confirmation of
248/251 that does not depend on either of our scoring scripts, and it gives you
honest per-function numbers for everything at once.

**Acceptance:** the `--all` output, pasted whole, plus your headline computed
from it. If it disagrees with 248/251, that disagreement is the most important
thing in your report — lead with it.

### 2. `setQuakeDead`, from the canonicalised diff

Get the real count first, then read the diffs and work from them. Your existing
diagnosis stands and is a good starting point: the draft hoists a `0` across
`UnKnownScoreSet` into non-volatile `r29` and grows the frame to `0x40`, where
the target tests `searchBaseByID`'s `r3` directly and persists nothing.

Two things worth knowing from the `revisePos` close:

- **A spurious callee-saved register and a word-count gap are usually the same
  fact.** Each saved GPR costs a `stw`/`lwz` pair — 2 words — plus frame. Your
  352-word draft against a 340-word target, with an extra non-volatile in play,
  fits that arithmetic. Check it before hunting for missing content.
- **When the registers are right and only the order is wrong, change the
  addressing form, not the statement order.** Nine declaration permutations on
  `revisePos` all bottomed out at the same number; a `const f32 *` cursor with
  `[0]/[1]/[2]` indexing closed it, because MWCC groups indexed loads off one
  base and schedules independent offset reads separately.

**Acceptance:** the canonicalised diff count before and after, ≥3 variants, and
the frame size and non-volatile set for each. Report the failures too.

### 3. The landing manifest

This is the half I cannot do without you, and it is worth as much as the
function. Blocker #4 is "promote 7 shared headers", and I have to do that
promotion by hand against headers other units depend on.

For **each** of the seven headers in `scratch/gemini_round24/include/`, give me:

- the **exact diff** against the real header in `include/` or `source/` —
  unified diff, not a description;
- for every symbol you added or changed, the **retail address and size** from
  the symbol map, as you did in the round-27 audit;
- whether the change is **additive** (new declaration only) or **modifies an
  existing declaration** — the second kind can break other units and I need
  those flagged separately and listed first;
- any header where your shadow copy has drifted from the real one for a reason
  unrelated to this unit. Say so plainly; I would rather drop a hunk than
  promote a hack.

**Acceptance:** seven sections, each with a real diff. A header with an empty
diff is a fine answer — say "identical, nothing to promote".

### 4. `__sinit` once more

You moved `.data` again if step 2 changes anything. Report its diff count
explicitly whatever it is, as you did last round.

`initializeState_Jump` / `BigJump` are closed as bounded negatives. Do not
reopen them.

---

## Constraints

Continue in `scratch/gemini_round24/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `tools/**`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`**, and do not attempt the landing itself. I will do
the header promotion and the five-binary verification — your manifest is what
makes that safe.

---

## Reporting

- The `fndiff.py --all` baseline, pasted whole.
- Headline computed once from it.
- **GAINED and LOST by name**, including the artifact-matched pair by name.
- `setQuakeDead`: canonicalised diff count before and after, per variant.
- **`__sinit`'s diff count**, explicitly.
- The seven-header landing manifest.
- `poolcheck.py` output.
- `NOT REACHED`, if anything.
