# Work order — round 31 (LARGE)

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

This brief is deliberately much larger than the last few. You have shown you
follow a narrow brief precisely; the open question is whether that holds at
volume. **Eight numbered tasks. Work them in order. Partial completion is
expected and fine — an honest "T1–T5 done, T6–T8 not reached" is a good
outcome. Inventing coverage is not.**

---

## Round 30 was blocked, the blocker was self-inflicted, and I unblocked it

You reported every variant as `Compiled: NO` and attributed it to *"the current
shared headers are incompatible with this old standalone draft."* You were right
to report the failure rather than fake a result. **But the diagnosis was wrong.**

Your compile resolved `game/bases/d_bg_ctr.hpp` to the **real** header in
`include/`, not to your shadow. The error trace says so: it enters via
`d_bg_ctr.hpp:3`, and line 3 of the *real* header is
`#include <game/bases/d_actor.hpp>`. Your shadow's include is on line 5, behind
the `class dBg_ctr_c;` forward declaration that exists precisely to prevent that
error.

**You omitted the shadow from the include path.** I compiled your own
`calc_order.cpp` unchanged, with the shadow passed as `extra_inc`, and it built
first time:

    harness.compile_draft(src, obj,
        extra_inc=['scratch/round31/d_bg_ctr/shadow'],
        module='wiimj2d')

Nothing is wrong with the tree, the headers, or the harness. **A whole round was
spent on a missing include path.** Build the script once in T0 below and never
hand-roll the invocation again.

### I ran your three variants for you. Both of my hypotheses were wrong.

    target        125 words, frame 0x60, FPR saves [31,30]
    v1 (true)     125 words, frame 0x50, FPR saves [31,30,29]
    calc_store    129 words, frame 0x60, FPR saves [31,30,29,28]   <- worse
    calc_order    125 words, frame 0x50, FPR saves [31,30,29]      <- same as v1
    calc_both     125 words, frame 0x50, FPR saves [31,30,29]      <- same as v1

**Store-before-call made it worse. Trig-call order changed nothing.** Both were
my ideas and both are now measured negatives — do not retry either. Record them.

What the numbers do say: the target reaches a **larger** frame (`0x60`) while
saving **fewer** FPRs. It is not carrying an extra register; it is carrying
extra *stack locals*. You are short roughly `0x10` of genuine local storage, not
long one register.

---

## A correction about round 29 that you need before you start

Round 29's `calc_v1_decl_order.o` and `.txt` are **byte-identical to round 27's**,
while its `.cpp` differs by 45 bytes and is *newer than the object*. The `.cpp`
in `scratch/round29/` is not the baseline — **it is already your inlined
variant**, with `cxs` removed and `corner0Y`/`corner1Y` added.

So the round-29 line *"Baseline compiled: YES. 125 words"* was round 27's
artifact copied forward, not a fresh compile of the file on disk. I checked the
numbers against those `.txt` files and did not re-compile, so I missed it and
called the round clean. That was my audit gap as much as your reporting error.

It then propagated: round 30 copied that file as its "baseline", so your entire
round 30 was built on the variant rather than the baseline.

**The true baseline is `scratch/round27/d_bg_ctr/calc_v1_decl_order.cpp`.** I
verified it: 125 words, frame `0x50`, FPR `[31,30,29]`. Use that file and no
other.

**New standing rule: every number you report must come from an object you
compiled in this round's directory.** Never copy a `.o` or a `.txt` forward. If
you want a baseline figure, recompile it. Two cheap habits that make this
checkable: write each variant to its **own filename**, and never edit a file you
have described as "unchanged".

---

## Round 31 — the work list

Everything in `scratch/round31/d_bg_ctr/`. Copy in `target.txt`, `diff_ctr.py`
and the `shadow/` tree from `scratch/round28/d_bg_ctr/`.

### T0 — build the harness once, and prove it

Write `scratch/round31/d_bg_ctr/build.py` exposing one function that takes a
source filename, compiles with `extra_inc=[<round31 shadow>]` and
`module='wiimj2d'`, disassembles, and returns `(words, frame, gpr_saves,
fpr_saves)` for a named function. Every later task calls it.

**Acceptance:** compile `scratch/round27/d_bg_ctr/calc_v1_decl_order.cpp` and
report **125 / 0x50 / FPR [31,30,29]**. If you do not get exactly that, stop and
report it — every task below depends on this being right.

### T1 — `calc` (+4): find the missing stack local

Target `125 / 0x60 / [31,30]`; true v1 `125 / 0x50 / [31,30,29]`. The word count
already matches; the shape does not. The target has ~`0x10` more local stack and
one fewer saved FPR.

Read the target's stack usage directly — which `r1+` offsets it writes that you
do not — and give the function the local it is missing so the third FPR is not
needed. **Do not** retry store-before-call or trig ordering.

**Acceptance:** at least three named variants, each with words/frame/saved-set.

### T2 — `fn_80080670` (−3)

Same family: draft saves `f31/f30`, target saves neither, both frame `0xB0`.
Apply whatever T1 learns. **Acceptance:** ≥2 variants measured.

### T3 — `fn_80080E40` (−4)

The draft-only gate deletion was correct and is done; you are now 4 words short
of real content. Target keeps `r31=idx`, `r30=dir`, `r29=this`, saves `r28..r31`,
does the `0xDC` test before `m_d4`, and calls `dBc_c::getActorKind()` **twice**.
Your draft saves only `r31/r30/r29` — it is short a register *and* the content
that needs it. **Acceptance:** ≥2 variants; report the GPR saved-set each time.

### T4 — `addDokanMoveDiff` (−7)

Target `87 / 0x60 / FPR [31,30]`; draft `80 / 0x50 / FPR [31,30,29]` — the same
shape mismatch as `calc`, so T1's answer probably transfers. Note your round-28
`target_math_dokan` reached 88 words; rebuild it in this round's directory (do
not copy it) and confirm the figure before building on it.
**Acceptance:** ≥2 variants, including a fresh `target_math_dokan`.

### T5 — `fn_8007FFA0` (−8)

Missing content, not register choice. Target frame `0x50`, `_savegpr_27`, one
FPR; it stores intermediates at `r1+0x10` and `r1+0x14` before the parent
accumulation and keeps the actor in `r27`. Write those stores and lifetimes.
**Acceptance:** ≥2 variants measured.

### T6 — `revisePos` (72/72)

You have two measured negatives here (target-read-order rewrites, codegen
neutral) and store-before-call is now a third. **One** further attempt, then
record it as a bounded negative and stop. **Acceptance:** one variant, or a
written statement that you are closing it as a bounded negative.

### T7 — `fn_80080900` (−48)

Target `0x170 / _savegpr_20`; draft `0xD0 / _savegpr_22`. Your liveness table
from round 26 is still the best artifact on this function. Round 28 found that
merely declaring the objects gets them optimised away — so make each one
**participate in a real store or call**. One serious pass.
**Acceptance:** one compiled attempt with its frame and `_savegpr` reported,
whatever they are.

### T8 — canonical fold

If any function reached a match, fold it into a canonical `d_bg_ctr.cpp` in
`scratch/round31/`, run `diff_ctr.py`, and report the new MATCHED count. If
nothing matched, say so and skip.

---

## Constraints

Work only in `scratch/round31/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `GEMINI_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`** — the tree is green and a concurrent build destroys
that.

---

## Reporting

One table, one row per compiled variant:

| Task | Variant file | Words (target/draft) | Frame (T/D) | GPR saves (T/D) | FPR saves (T/D) | Diffs |

Diff count last, and only where frames agree — where they differ, say so.

Then per task, two lines: what you tried, and what the measurement says.

- **Every row must come from an object compiled in `scratch/round31/`.** No
  copied artifacts.
- **GAINED / LOST by name**, matched-status changes only. `none` is fine.
- **Tasks not reached: list them explicitly as not reached.** That is a good
  answer and I would rather have it than padding.
- `poolcheck.py` on any canonical object, via the CLI:

      python tools/auto_decomp/poolcheck.py --module wiimj2d \
          --obj <obj> --txt <disasm> scratch/round31/d_bg_ctr/target.txt

- Drop the `Offset-perturbing` field. Fourth time of asking.
