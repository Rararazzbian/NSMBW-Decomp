# Work order — round 30

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 29: you did everything right. My hypothesis was wrong.

Verified against your artifacts before writing this. You:

- started from `calc_v1_decl_order.cpp`, as instructed;
- compiled it unchanged first and reproduced the gate exactly — **125 words,
  frame `0x50`, saves `f31 f30 f29`**;
- made precisely the change I specified and no other;
- tried two further shapes, then **stopped**, as instructed;
- reported the saved-register set as the scoreboard;
- restored the correct `GAINED`/`LOST` definition — *"none (no matched-status
  change)"* — after last round's drift;
- touched exactly one function.

Every number reproduces. **That is a clean round and the process question is
settled.**

**The result is a negative, and it is my error, not yours.** I predicted that
inlining `cxs` at both use sites would shorten the live range and drop `f29`.
It did the opposite — both variants came back at **129 words with `f28` back**,
i.e. the baseline six-save shape. Recorded: **naming that temporary is better
than inlining it here**, and the def-point lever does not reach this residual.
A refuted prediction that moves the number is still information; this one moved
it the wrong way and that is worth knowing.

Drop the `Offset-perturbing` line — third time of asking, and it has never
carried information.

---

## I looked again, and the real difference is not the live range. It is order.

Two findings, both single source edits, both from the same side-by-side.

### 1. The two trig calls are in the opposite order

    target:  bl CosFIdx  ->  fmr f31, f1  ->  bl SinFIdx
    v1:      bl SinFIdx  ->  fmr f31, f1  ->  bl CosFIdx

The target computes **cosine first, then sine**. Your draft does sine first.
Whichever is computed first has to survive the second call, so the call order
decides what gets parked in a callee-saved register. **Swap them** so cosine is
evaluated first.

Note the target's `f30` is not a computed value at all — it is a constant load,
`lfs f30, SYM1@sda21(r0)`. Only **one** computed value crosses a call in the
target. Three cross one in yours.

### 2. Your three products are held across the `revisePos()` call

This is the bigger one.

    v1      66  fmuls f29, f5, f31        target  77  fadds f31, f10, f1
            70  fmuls f30, f3, f1                 80  stfs  f31, 0x74(r30)
            73  fmuls f31, f3, f31
           ...                                   ...
           104  bl revisePos                     106  bl revisePos

The target **stores each result to memory as soon as it computes it**, before
calling `revisePos()`. Your draft computes all three products, holds them in
registers across the `revisePos()` call, and stores afterwards — which is
exactly why they need callee-saved registers at all.

**Move the writes to `mScratch[...]` above the `revisePos()` call**, so each
value is computed and immediately stored. That should let all three live in
volatile registers and drop `f29`, `f30` and `f28` together.

---

## Round 30 — order of work

Still `calc` first, still from `calc_v1_decl_order.cpp` in `scratch/round30/`.
You have earned wider scope back, but finish `calc` before moving on.

1. **`calc`** — finding 2 first (store before the call), since it explains three
   saves rather than one. Then finding 1 (swap the trig call order). Then both
   together. **Report the saved-register set for each.** If `calc` matches, fold
   it into a canonical `d_bg_ctr.cpp`, re-run `diff_ctr.py`, report the MATCHED
   count.
2. **`fn_80080670` (−3)** — same family: your draft saves FPRs the target does
   not. Check whether it too is holding values across a call that the target
   stores first. This is the cheapest test of whether finding 2 generalises.
3. **`revisePos` (72/72)** — you have two measured negatives here already
   (target-read-order rewrites, codegen-neutral). Try it once through the
   store-before-call lens instead, then leave it.
4. **`addDokanMoveDiff`** — you reached 88 against target 87, the closest it has
   been. One word. Worth a look now that `calc` may have taught you the shape.

Nothing else. Do not reopen `fn_80080900`.

Work in `scratch/round30/`. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `GEMINI_*`, `CODEX_HANDOFF.md`, or
`HANDOFF.md`. **Do not run `ninja`, `configure.py`, `progress.py` or `land.py`.**

---

## Reporting

Per variant: word count, **saved-register set**, frame size. Diff count last,
and only where frames agree.

**GAINED / LOST by name**, matched-status changes only — exactly as you did in
round 29.

`poolcheck.py` on the final object.
