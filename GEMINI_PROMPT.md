# Work order — round 33

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 32: the fold happened, and I verified it

I re-scored your object independently. `setQuakeDead` is **85 words, frame
`0x30`, GPR `[30,31]`, one instruction differing.** That is real and it is in the
object where I can see it. Thank you for folding first.

Eighteen variants, the mechanism mapped, the `l_EnMuki` declaration checked
against the header rather than assumed. All good work.

---

## But the `(mUnk770 >> 31)` null cannot be the source, and I can prove it

**`setShellDead`, `setFumiDead`, `setFireDead` and `setStarDead` all match
byte-exact, and every one of them contains the identical block:**

    mUnk792 = 0;
    mUnk790 = 0;
    <a call>
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }

Plain `nullptr`. And retail's `setShellDead` emits exactly what you want:

    21  li r0, 0x0        <-- the two sth stores
    ...
    42  li r3, 0x0        <-- a FRESH zero for the null arm
    44  bl searchBaseByID__10fManager_cF9fBaseID_e

So the literal null is correct source, it compiles correctly four times in this
same file, and `>> 31` is a workaround for a defect that is **somewhere else in
`setQuakeDead`**.

This is the second time this unit has been steered by a diff count toward a
shape that cannot be the source — the guard-`if` was the first. The rule from
that one applies again: **the listing chooses the shape.** Here you have
something even better than a listing: four sibling functions with the same
construct that already match. **When a construct fails in one function and
matches in four others in the same file, the construct is not the bug.**

Note also that register availability is not the explanation. Retail's
`setQuakeDead` saves only `[30,31]`, so `r29` is free there too, and it still
materialises a fresh `li r3, 0`.

**Revert the null to `nullptr` and go and find the real difference.**

## Where to look

`setQuakeDead` and `setShellDead` differ in ways you can enumerate. Two are
worth checking before anything else:

**1. The death-info struct.** Your source has:

    sDeathInfoData deathData = l_dieQuake;

and the draft emits `lis r11, l_dieQuake@ha`. **Retail emits
`lis r11, "@70611_802F0C40"@ha`** — an anonymous pooled symbol, not a named
global. `setShellDead`, which matches, uses a compound literal:

    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, ... };

An anonymous pool entry is what a compound literal produces. A named `l_dieQuake`
global is what a named global produces. **The retail symbol says compound
literal.** My differ classes this as a naming artifact because one side is
anonymous, so it has been invisible in the diff count — but it is a genuine
source-level difference, and it changes what the constant pool for this function
contains. Reconstruct the literal from the retail data at `0x802F0C40` and use
it inline, exactly as `setShellDead` does.

**2. The unconditional call.** In `setShellDead` the score call sits inside
`if ((u32)playerNo <= 3)`. In `setQuakeDead` `UnKnownScoreSet` is unconditional.
That changes whether the zero constant is live on a single straight-line path
into the merge. I am not asking you to make it conditional — retail's is
unconditional — but it is the structural difference that most plausibly explains
why the same construct behaves differently here, so keep it in mind when reading.

**Ruled out, do not re-run:** a `static inline` helper containing the null test,
in three forms — `if`/`return nullptr`, `if`/`return 0`, and an internal ternary.
All three inline and then merge exactly as the direct ternary does; inlining is
not a scope barrier for this.

---

## Completeness

All three items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Round 33 — order of work

### 1. `setQuakeDead` with `nullptr` restored

Revert the null. Then fix the death-info literal, rebuild, and report. If that
alone does not do it, diff `setQuakeDead` against `setShellDead` **statement by
statement** and report every structural difference you find, with a build for
each one you can test. The answer is in that list.

**Acceptance:** the `nullptr` restored; the death-info question answered with a
build; ≥3 further variants; and for each, words / frame / non-volatile set /
whether `li r3,0` is present.

Keep your `>> 31` object as a separate file so we do not lose the 85-word
result, but the canonical source uses `nullptr`.

### 2. The Jump twins

`l_EnMuki` is already `extern const s8` — that lever is spent, and confirming it
from the header rather than guessing was the right move. So go to the other one:
`speed` itself. Assign `.x` and `.y` component-wise in each arm of the
`if (flag)` block instead of copying the whole `mVec2_c`, and try the same
compound-literal question — check whether `mpParamJump->mJumpSpeed1` is what
retail actually reads, or whether the retail listing points at a pooled
anonymous datum there too.

Start from `scratch/claude_kokoopa/k6_sx_late.cpp` (5 diffs). Do not re-run the
24 shapes in the round 31 brief or the 8 in your round 32 report.

**Acceptance:** ≥4 variants, both twins scored each time, with the observed
f2/f3/f4 allocation.

### 3. Fold and verify

Whatever is best at the end, fold it, rebuild, `fndiff.py --all`, `poolcheck.py`.

---

## Constraints

Continue in `scratch/gemini_round24/`. You may read `scratch/claude_kokoopa/`
but do not write there. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `tools/**`, `QWEN_*`, `CODEX_HANDOFF.md`,
or `HANDOFF.md`. **Do not run `ninja`, `configure.py`, `progress.py` or
`land.py`**, and do not attempt the landing.

---

## Reporting

- **Lead with your best number.**
- The death-info literal question, answered with retail evidence.
- `setQuakeDead` variant table, five columns.
- Jump/BigJump table, both twins, with allocations.
- `fndiff.py --all` after folding, pasted whole.
- **GAINED and LOST by name**, including the artifact-matched pair by name.
- `poolcheck.py` output.
- `NOT REACHED`, if anything.
