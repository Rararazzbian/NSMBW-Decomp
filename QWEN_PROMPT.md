# Work order — round 29

**One function. One starting file. One change.** Read this whole brief before
touching anything; it is short on purpose.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Why this round is narrow

Round 28 compiled 13 objects and moved the deliverable by nothing.
`scratch/round28/d_bg_ctr/d_bg_ctr.cpp` is byte-identical to round 27's.

The round-28 brief asked you to start from `calc_v1_decl_order` — the 125-word
variant that had already eliminated the spurious `f28` — and remove `f29` from
it. Instead every round-28 object went back to the 129-word baseline carrying
**both** `f28` and `f29`, and the one `calc` attempt reached 139 words. That is
the second round running that your own best result was discarded, and the first
where it was discarded against an explicit instruction.

So: nothing else this round. Breadth is not the problem.

**Also, correct the scoreboard.** You reported *"GAINED: every function now has
a fresh compilable experiment"* and *"GAINED: `target_math_dokan` reached 88
words"*. Neither is a gain. **GAINED and LOST mean one thing only: a function
that changed matched status.** The honest entry for round 28 was `GAINED: none,
LOST: none` — which you also wrote, three lines later. Your numbers were all
accurate and reproduced exactly; the framing was not. Do not redefine the metric.

---

## The task: `calc`, from `calc_v1_decl_order.cpp`, remove the `f29` save

**Start file, exactly:** `scratch/round27/d_bg_ctr/calc_v1_decl_order.cpp`
Copy it to `scratch/round29/d_bg_ctr/` and work there. Do **not** start from
`d_bg_ctr.cpp`, and do not re-run the round-27 or round-28 experiments.

State of play:

    target      125 words, frame 0x60, saves f31 f30 r31 r30  and NO other FPR
    v1          125 words, frame 0x50, saves f31 f30 f29 r31 r30
    baseline    129 words, frame 0x60, saves f31 f30 f29 f28 r31 r30

v1 is one spurious callee-saved FPR from the target's shape. **`f28` is already
gone. Remove `f29` the same way.**

### I have located `f29` for you

In `calc_v1_decl_order.txt`, `f29` is written once and read twice, far apart:

     66  fmuls f29, f5, f31      <- computed here
     77  fsubs f1, f30, f29      <- read here
     83  fsubs f6, f6, f29       <- and here

A value defined once and consumed at two distant points is exactly what forces
MWCC to park it in a **callee-saved** register — that is the def-point rule in
`AGENT_CONTEXT.md` ("a def-point is not free"), and lever 13, the read-side
def-point.

**The target does not hold that product at all.** Its entire arithmetic body
runs in volatile registers — every `fmuls` lands in `f13`, `f12`, `f9`, `f8`,
`f5`, `f4`, `f3`, `f2`, and nothing survives long enough to need saving:

     59  fmuls f13, f7, f31       67  fmuls f5, f2, f31
     61  fmuls f12, f4, f1        68  fmuls f4, f2, f1
     62  fmuls f9, f4, f31        70  fmuls f3, f0, f1
     64  fmuls f8, f7, f1         71  fmuls f2, f0, f31

**The change:** find the named local in `calc_v1_decl_order.cpp` holding that
`f5 * f31` product and used at both subtraction sites, and **write the
expression inline at each use instead of storing it in a variable.** Shorten its
live range to nothing and it should stay volatile.

That is the change. One edit.

---

## What to do, in order

1. Copy `calc_v1_decl_order.cpp` into `scratch/round29/d_bg_ctr/`. Compile it
   unchanged first and confirm you reproduce **125 words, `f29` present**. If you
   cannot reproduce that, stop and report it — everything below depends on it.
2. Make the single change above. Compile.
3. Report the **saved-register set** and frame. That is the scoreboard, not the
   diff count — the frames differ, so the diff count is measuring displacement.
4. If `f29` is gone and the frame is `0x60`, diff it against target and report
   what is left. If `calc` matches, fold it into a canonical `d_bg_ctr.cpp`,
   re-run `diff_ctr.py`, and report the new MATCHED count.
5. If the change does not remove `f29`, try **at most two** further shapes for
   shortening that value's live range, and report the saved-register set of each.
   Then stop.

**Do not touch any other function.** Not `revisePos`, not `addDokanMoveDiff`,
not `fn_80080900`. If you finish early, stop early — a short report that closes
`calc` is the best outcome available this round.

Work in `scratch/round29/`. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `GEMINI_*`, `CODEX_HANDOFF.md`, or
`HANDOFF.md`. **Do not run `ninja`, `configure.py`, `progress.py` or `land.py`.**

---

## Reporting

Short. One function.

- For every variant compiled: word count, **saved-register set** (which GPRs,
  which FPRs), frame size. Diff count last, and only where frames agree.
- **GAINED / LOST by name**, under the definition above — matched-status changes
  only. `none` is a perfectly good answer and has been the true one twice.
- `poolcheck.py` on the final object, via the CLI:

      python tools/auto_decomp/poolcheck.py --module wiimj2d \
          --obj <obj> --txt <disasm> scratch/round29/d_bg_ctr/target.txt
