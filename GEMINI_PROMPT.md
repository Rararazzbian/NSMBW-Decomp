# Work order — round 32

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 31: your best result of the project, and you buried it

You found a `setQuakeDead` variant at **85 words / frame `0x30` / GPR `[30,31]`
with one instruction differing** — `srwi` or `lhz` where retail has `li r3, 0`,
jumping into the same shared `cmpwi`. That is one instruction from closing a
landing blocker, and it appeared as a sub-bullet under item 2.

**Lead with the best number you have.** A one-instruction gap on a blocker is
the headline; the surrounding sweep is supporting detail.

You also produced the `l_EnMuki` symbol fact — `.sdata2:0x8042C480`, size 2,
byte data, so `s8[2]` — which is exactly the kind of retail evidence I asked
for. You then did not test it. **Changing that declaration is an untried lever
and you are the one who found it.**

Third round running, your best variant is not in the object. I re-scored
`scratch/gemini_round24/draft_disasm.txt` and it is still the 88-word,
frame-`0x40`, `r29` baseline. **Folding is not optional bookkeeping** — it is how
I verify your number and how the unit accumulates. From now on, fold first,
then sweep, and if a fold loses something, hand me both objects.

---

## Your CSE diagnosis is wrong. Here is what MWCC is actually doing.

You have written for three rounds that MWCC common-subexpressions the constant
zero across `UnKnownScoreSet` because it counts three uses against a
two-instruction threshold. I tested that directly. **It is not the mechanism.**

Chaining the stores (`mUnk790 = mUnk792 = 0;`), reversing the chain, copying
(`mUnk790 = mUnk792;`), a typed local (`s16 z = 0;`), and `(u16)0` casts all
produce **byte-identical output** — 88 words, frame `0x40`, `r29` saved. Reducing
the number of literal zeros changes nothing, so the count is not the trigger.

Read the draft's own listing:

    li    r29, 0x0                  <-- the store constant, placed in a
    sth   r29, 0x792(r30)               CALLEE-SAVED register
    sth   r29, 0x790(r30)
    bl    UnKnownScoreSet__11dScoreMng_cFP8dActor_cUlff
    lwz   r3, 0x770(r30)
    cmpwi r3, 0x0
    bne   .L1408
    b     .L1410                    <-- no `li` at all: r29 is ALREADY 0
    .L1408:
    bl    searchBaseByID__10fManager_cF9fBaseID_e
    mr    r29, r3                   <-- merge into r29
    .L1410:
    cmpwi r29, 0x0
    beq   .L1420
    mr    r3, r29                   <-- and copy back out for the call
    bl    deleteRequest__7fBase_cFv

versus retail:

    li    r0, 0x0                   <-- VOLATILE register, dead at the call
    sth   r0, 0x792(r30)
    sth   r0, 0x790(r30)
    bl    UnKnownScoreSet__11dScoreMng_cFP8dActor_cUlff
    ...
    li    r3, 0x0                   <-- fresh zero, merge stays in r3
    b     .L_800A9B2C
    ...
    cmpwi r3, 0x0                   <-- no mr in, no mr out
    beq   .L_800A9B38
    bl    deleteRequest__7fBase_cFv

**MWCC is choosing `r29` as the ternary's merge register** because the store
constant is already zero, so the null arm becomes free — and it pays for that
with a callee-saved register, two `mr` instructions and `0x10` of frame. The
arithmetic is exact:

    + 2 words   mr r29,r3  and  mr r3,r29
    + 2 words   stw r29 / lwz r29
    - 1 word    the li r3,0 it avoided
    = + 3 words   88 against 85

**So the goal is not to defeat a CSE. It is to stop MWCC merging the ternary
into the register that holds the store constant, and keep the merge in `r3`.**
Your `(mUnk770 >> 31)` and `(u32)mUnk790` variants work because a computed zero
cannot be served by `r29` — which is why they hit 85 words with the right frame
and register set. They are on the correct road. They just spend the instruction
on the wrong opcode.

I also got a variant to 86 words with the right frame and registers, saved at
`scratch/claude_kokoopa/q5_null_then_if.cpp` — a null-initialised local plus a
plain `if`. It moves the merge to `r0` instead of `r29`, which is nearly right,
but it loses the `li r3,0`/`b` pair and costs two `mr`s. Read its diff; the
shape it reaches is informative even though it is a word over.

**These are ruled out — do not re-run them:** chained/reversed/copied zero
stores, `s16` and `u16` typed zero locals, `(fBase_c *)0`, `static_cast`,
`if (fBase_c *base = ...)` declaration-in-condition, and `if/else` with an
empty then-branch. All eight give the same 88/`0x40`/`r29` object.

---

## Completeness

All three items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Round 32 — order of work

### 1. Fold your best `setQuakeDead` variant NOW, before anything else

The 85-word one. Rebuild, run `fndiff.py --all`, confirm 248 still match, and
report the count. Then continue.

### 2. `setQuakeDead`: find the zero that costs one `li`

You need a null that MWCC cannot serve from `r29` **and** that it materialises
with a single `li r3, 0`. Your two working expressions cost a `srwi` and an
`lhz` respectively. Untried directions:

- a null whose *type* differs from the ternary's result, forcing a conversion
  node that MWCC then folds to `li` — e.g. a `void *` null, or a null of an
  unrelated class pointer, cast at the assignment;
- forcing the store constant out of a callee-saved register instead of changing
  the null at all. If `li r0,0` is used for the stores as in retail, the merge
  has nothing to reuse. Anything that makes that constant obviously short-lived
  is worth a build;
- making `base` used in a way that pins it to `r3` — the retail listing never
  copies it out, so whatever retail's source is, `base` never needs a register
  of its own.

**Acceptance:** ≥4 variants, each with words / frame / non-volatile set /
whether `li r3,0` + `b` into a shared `cmpwi` is present / the diff count.

### 3. The Jump twins: test the `s8` declaration

You established `l_EnMuki` is `s8[2]` in `.sdata2`. If it is currently declared
as `int[]` or `s16[]` in the draft, **the cast idiom changes** — a `lbz`/`extsb`
into an int-to-float differs in register cost from a word load, and the
allocation inversion I mapped last round is entirely about when those two FP
registers are taken. Fix the declaration to match retail and re-measure both
twins.

If that does not do it, the other untried lever is `speed` itself: the two
`mVec2_c` assignments in the `if (flag)` block above. Try assigning `.x` and
`.y` component-wise in each branch instead of copying the whole struct.

Start from `scratch/claude_kokoopa/k6_sx_late.cpp`, which is the 5-diff form.
**Do not re-run the 24 shapes listed in round 31's brief.**

**Acceptance:** the `l_EnMuki` declaration question answered with a build, plus
≥3 further variants, each reporting the observed f2/f3/f4 allocation for **both**
twins.

---

## Constraints

Continue in `scratch/gemini_round24/`. You may read `scratch/claude_kokoopa/`
but do not write there. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `tools/**`, `QWEN_*`, `CODEX_HANDOFF.md`,
or `HANDOFF.md`. **Do not run `ninja`, `configure.py`, `progress.py` or
`land.py`**, and do not attempt the landing.

---

## Reporting

- **Lead with your best number**, then the detail.
- Confirmation that the fold happened, with `fndiff.py --all` pasted whole.
- `setQuakeDead` variant table with the five columns above.
- Jump/BigJump table with both twins' counts and the f2/f3/f4 allocation.
- **GAINED and LOST by name**, including the artifact-matched pair by name.
- `poolcheck.py` output.
- `NOT REACHED`, if anything.
