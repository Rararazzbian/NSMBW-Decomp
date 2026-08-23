# Work order — round 33

**Read `AGENT_CONTEXT.md` first**, including the two new sections at the end:
*"Count diffs on CANONICALISED text"* and *"Register ALLOCATION order and load
EMISSION order are separate levers"*. They are the whole basis of this round.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## COMPLETENESS — read this before anything else

**Every task in this brief is in scope. Do all of them.**

- **If a task's details seem incomplete, that is not permission to skip it.**
  Make the most reasonable interpretation, do the work, state the assumption.
- **If something you need is missing** — a file, a figure, a tool — **do not
  drop the task.** Find the nearest valid substitute and say what you used.
- **A task is only "not done" if you ran out of budget**, and then name it in a
  `NOT REACHED` list. Silence is not an acceptable way to leave something undone.
- **A variant is new source written this round.** Recompiling an inherited file
  is a reference point, not an experiment, and does not count toward a minimum.

End with a **compliance table**: one row per task, `required` / `delivered` /
`met?`. I check it against your artifacts either way.

---

## Round 32, honestly

You delivered T0 and nothing else. Every other row was a round-28 file
recompiled, and your own compliance table said so in nine places. That
self-report was accurate and I would rather have it than a padded round — but
seven tasks were not attempted, and the reason given ("ran out of execution
budget") happened after the round had already spent its budget copying and
rebuilding nine inherited sources that you knew would not count.

**Do not open round 33 by recompiling last round's files.** There is one
baseline, it is already built, and you should start from a measurement of it.

### Your T0 was half right

Words and frames: **7 of 7 correct.** That is the hard half and you got it.

Register saves: **wrong on 5 of 7 rows.** You reported "none" for GPR saves on
`calc`, `revisePos`, `addDokanMoveDiff`, `fn_80080670` and `fn_80080E40`; all
five save registers explicitly. `revisePos`'s prologue is three `stw`s:

    /* 8008017C */  stw r31, 0x2c(r1)
    /* 80080180 */  stw r30, 0x28(r1)
    /* 80080184 */  stw r29, 0x24(r1)

You were right about `fn_8007FFA0` and `fn_80080900` — those two use
`_savegpr_27` and `_savegpr_20`, and reading the helper number as the first
saved register is correct.

**This is why the tooling task below comes first.** Reading prologues by hand is
where the errors were, and it is a solved problem.

---

## `revisePos` is MATCHED. Here is exactly how, because it is the method.

I closed it. **33 of 39 functions in `d_bg_ctr` now match**, zero regressions,
`poolcheck.py` clean across all 33. It took four steps and about ten minutes.

**Step 1 — canonicalise before counting.** Raw positional diff was 16. Ten of
those were names: six branch labels (`.L_800801F8` vs `.L_00000D38`), three
mangled call targets, one pooled float symbol. **The real count was 6.** You
have been steering on numbers inflated the same way in every function.

**Step 2 — read the six.** They were three `fsubs` and the loads feeding them:
same six addresses, same order, same arithmetic, **different registers**.

**Step 3 — fix allocation with declaration order.** Declarations were
`old, actor, actor, old, actor, old`; making every pair `old, actor` made all
six registers identical. **6 diffs → 4.**

**Step 4 — fix emission with the read form.** The last 4 were load order only.
Nine declaration orderings were tried and *every one* bottomed out at 4, because
allocation and emission both follow declaration order and the target needs them
opposed. What worked was changing how the memory is addressed:

    const f32 *op = (const f32 *)((const u8 *)this  + 0x94);
    const f32 *np = (const f32 *)((const u8 *)actor + 0xAC);
    f32 f4 = np[2] - op[2];
    f32 f2 = np[1] - op[1];
    f32 f1 = np[0] - op[0];

**Zero.** MWCC groups indexed loads off one base and hoists them together;
independent `rawF32(p, off)` calls it schedules separately. Same semantics,
different codegen shape.

**Carry this forward: when the registers are right and only the order is wrong,
stop permuting statements and change the addressing form.**

---

## T1 — use the differ, and report canonicalised counts only

`tools/auto_decomp/fndiff.py` is new and committed. It renumbers labels and pool
symbols per side, strips `fn_XXXXXXXX__<mangling>`, and prints words / frame /
GPR / FPR for both sides:

    python tools/auto_decomp/fndiff.py TARGET.txt DRAFT.txt FUNCTION -v
    python tools/auto_decomp/fndiff.py TARGET.txt DRAFT.txt --all

`--all` scores every function in the object in one call — run it after **every**
compile, because it is also how you prove you broke nothing.

**Acceptance:** an `--all` run against the round-33 baseline, pasted whole. It
must show 33 functions at `DIFFS 0`. If it does not, stop and tell me.

**Do not hand-read a prologue again this round.** `fndiff.py` prints the saves.

---

## The baseline

    scratch/round33/d_bg_ctr/d_bg_ctr.cpp     <- canonical, 33/39, already built
    scratch/round33/d_bg_ctr/build.py         <- repointed to round33
    scratch/round33/d_bg_ctr/shadow/          <- pass this in extra_inc, always
    scratch/round33/d_bg_ctr/target.txt

Work in `scratch/round33/d_bg_ctr/`. Every variant gets its own filename.

---

## The six remaining functions, measured by me this morning

| Function | words T/D | frame T/D | GPR T/D | FPR T/D | diffs |
|---|---|---|---|---|---|
| `calc` | 125 / **129** | 0x60 / **0x60** | [30,31] / [30,31] | **[30,31] / [28,29,30,31]** | 103 |
| `fn_8007FFA0` | 115 / 107 | 0x50 / 0x60 | `_savegpr_27` both | **[31] / [29,30,31]** | 103 |
| `addDokanMoveDiff` | 87 / 80 | 0x60 / 0x50 | [29,30,31] both | **[30,31] / [29,30,31]** | 65 |
| `fn_80080670` | 130 / 127 | 0xB0 / **0xB0** | [30,31] both | **none / [30,31]** | 115 |
| `fn_80080E40` | 121 / 117 | 0x20 / **0x20** | **[28,29,30,31] / [29,30,31]** | none both | 110 |
| `fn_80080900` | 256 / 208 | 0x170 / 0xD0 | `_savegpr_20` / `_savegpr_22` | [31] both | 206 |

Verify any row you intend to act on with `fndiff.py` rather than trusting me.

## Look at the FPR column. That is the round.

**Five of the six drafts save callee-saved registers the target does not**, and
in three cases that discrepancy is *exactly* the word gap:

- **`calc`** — two spurious FPRs (`f28`, `f29`). A plain `stfd`/`lfd` pair is
  2 words, so two spurious registers is **4 words**, and the draft is **+4**.
  Kill `f28` and `f29` and the function is the right length with the right
  frame. The frame already agrees at `0x60`.
- **`fn_8007FFA0`** — two spurious FPRs (`f29`, `f30`) = 4 words of prologue and
  epilogue, and `0x10` of frame, which is exactly the `0x60` vs `0x50` gap. The
  draft is 8 words short, so removing them leaves **12 words of missing
  content** to find, not 8. Do the register work first so the content figure is
  honest.
- **`fn_80080670`** — two spurious FPRs (`f30`, `f31`) against a target that
  saves none. Removing them costs 4 words from a draft already 3 short, so the
  real content gap is **7 words**.
- **`addDokanMoveDiff`** — one spurious FPR, and note the target's save idiom:
  `stfd f31` **plus** `psq_st f31, ..., 0, qr0`, restored with `psq_l` + `lfd`.
  That is 4 words per register, not 2, and it means the target is holding
  **paired singles** — two floats in one register. Frame 0x60 vs 0x50 is the
  extra pair slot. This function's shape is different from what you have drafted.
- **`fn_80080E40`** — the only one that runs the other way: the target saves
  `r28` and your draft does not. You need **one more long-lived value**, not one
  fewer.

**A spurious callee-saved register means a value is alive across a call that
should not be.** The proven levers, in order of hit rate on this project: move
the definition to the point of use; recompute instead of caching; let the value
die before the call by consuming it earlier; and — new this round — change the
addressing form so the loads regroup.

---

## Round 33 — the work list

### T2 — `calc`: kill `f28` and `f29` (**the round's priority**)

Words, frame and GPR all agree; two FPRs and four words are the entire gap, and
those two facts are the same fact. Find what is alive across the trig calls.

Ruled out, do not retry: store-before-call, trig-call ordering, whole-function
declaration reordering (139 words, worse). Those were all attempts to move the
*statements*. Try moving the *reads* instead.

**Acceptance:** ≥3 new variants, each with its `fndiff.py` line. Report the FPR
set for every one, even the failures — a variant that drops `f28` but grows is
progress and I want to see it.

### T3 — `fn_8007FFA0`: kill `f29` and `f30`, then find 12 words

Same shape as `calc` and it is a small static helper, so the live-range analysis
is tractable end to end. `stack_x` and `stack_y` survive three call sites in the
current draft — that is the obvious suspect and it is worth confirming or
killing outright.

**Acceptance:** ≥2 new variants + a statement of which value forces each save.

### T4 — `fn_80080670`: kill `f30` and `f31`

Frame already agrees at `0xB0`. **Do not** repeat `t2_local_vec` (129/0xC0).

**Acceptance:** ≥2 new variants.

### T5 — `fn_80080E40`: find the value that needs `r28`

The reverse problem. Something in the target lives across more of the function
than anything in your draft does. Four words short, one GPR short.

**Acceptance:** ≥2 new variants.

### T6 — `addDokanMoveDiff`: the paired-single question

Before writing any variant, answer one question from the target listing: **which
two floats share `f30`, and which share `f31`?** Read the `psq_st`/`psq_l` offsets
and the uses in between. Put the answer in your report. Then write variants that
give MWCC a reason to pair them — adjacent `mVec3_c` components assigned
together is the usual cause.

**Acceptance:** the pairing answer + ≥2 new variants.

### T7 — `fn_80080900`: one pass, and a word budget

48 words short and a frame less than half the target's. Do not chase the diff
count. **Produce a written estimate of where the 48 words are** — which
conditional, which loop, which call is absent — backed by the target listing,
then one compiled attempt against your best guess.

**Acceptance:** the estimate + one compiled attempt with `fndiff.py` output.

### T8 — fold and re-verify

Fold every improvement into `scratch/round33/d_bg_ctr/d_bg_ctr.cpp`, rebuild,
and run `fndiff.py --all` plus `poolcheck.py`:

    python tools/auto_decomp/poolcheck.py --module wiimj2d \
        --obj scratch/round33/d_bg_ctr/d_bg_ctr.o \
        --txt scratch/round33/d_bg_ctr/d_bg_ctr.txt \
        scratch/round33/d_bg_ctr/target.txt

**33 functions must still be at `DIFFS 0`.** If folding two improvements
together loses one, say so and hand me the two separately.

---

## Constraints

Work only in `scratch/round33/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `tools/**`, `GEMINI_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`.**

---

## Reporting

| Task | Variant file | New this round? | Words T/D | Frame T/D | GPR T/D | FPR T/D | Diffs |

Every figure from `fndiff.py`. Only new rows count toward a minimum.

- The `--all` baseline run, pasted whole, before any variant work.
- **GAINED / LOST by name**, matched-status changes only. Nothing else belongs
  in those two sections.
- `NOT REACHED`, if anything.
- `poolcheck.py` output on the folded object.
