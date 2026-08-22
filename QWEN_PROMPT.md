# Work order — round 28

**Read `AGENT_CONTEXT.md` first.** One new section came out of your round 27 and
it changes how you should score `calc`.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 27 compiled. Five objects, and every number reproduces.

I checked the artifacts before the report this time, as I did last round:

    scratch/round27/d_bg_ctr/*.o        5 objects, all fresh
    d_bg_ctr.cpp                        genuinely changed
    diff_ctr.py                         MATCHED: 32  DIFFER: 7  MISSING: 0

Every figure in your table reproduces exactly, including `fn_80080E40` at 117
and all three `calc` variants at 125 / 125 / 129. That is the round-26 problem
fixed.

**Your poolcheck blocker is real and I have the fix.** `harness.poolcheck` does
not exist — your copied wrapper calls an entry point that is gone. Reporting it
as a tooling blocker instead of inventing a clean result was the right call. The
working invocation is the module's own CLI:

    python tools/auto_decomp/poolcheck.py --module wiimj2d \
        --obj <your>.o --txt <your>_disasm.txt <your>/target.txt

I ran it against your round-27 object: **7 constants compared, 0 mismatched, 0
unresolved.** You are clean. Use that command from now on.

---

## You threw away your two best results. `calc` is closer than you think.

You wrote: *"The 125-word variants still differ in instructions; matching length
alone did not close `calc`. This confirms the residual is not just word count."*
True as stated, and it led you to the wrong conclusion.

Here is what the prologues actually show. **The target saves `f31`, `f30`, `r31`,
`r30` — and no other FPR at all:**

    baseline   129 words, frame 0x60, saves f29 AND f28 spurious  -> 103 diffs
    v1 / v2    125 words, frame 0x50, saves f29 spurious          -> 121 diffs
    target     125 words, frame 0x60, saves NEITHER

**v1 and v2 eliminated one of the two spurious callee-saved FPRs.** That is
exactly your missing 4 words, and it is real progress on precisely the axis I
pointed you at. The diff count rose from 103 to 121 only because shrinking the
frame by `0x10` displaced every stack offset below it — 121 lines "differ"
because `stfd f31, 0x50(r1)` became `stfd f31, 0x40(r1)`, and so on down the
function. That is one defect wearing 121 costumes.

`AGENT_CONTEXT.md` already carried *"never revert a change purely because the
count went up"*; it now also carries the specialisation: **when frame sizes
differ, score on the saved-register set, not the diff count.**

So `calc` is one spurious save from home. Take `calc_v1_decl_order` — not the
baseline — and apply the same lever again to kill `f29`. Note the target reaches
`0x60` of frame *without* those saves, so it has roughly `0x10` more genuine
local stack than you do: the target does `addi r3, r1, 0x20` where your variant
does `addi r3, r1, 0xc`. You are short a stack local, not carrying an extra one.

---

## `fn_80080E40`: the deletion was right, and it behaved exactly as predicted

121 target, 124 before, **117 after**. You removed the draft-only gate and the
function went from 3 words long to 4 words short.

That is the same pattern as the `CosIdx` swap two rounds ago and it is now a
rule in `AGENT_CONTEXT.md`: **a correct fix that overshoots has uncovered
content you never wrote.** The gate was genuinely absent from retail, the
deletion was genuinely correct, and the honest 117 has replaced a flattering
124 that was two errors cancelling. Do not restore the gate. Find the missing 4
words — you already noted the target's parameter roles (`r31=idx`, `r30=dir`,
`r29=this`) and that it performs the `0xDC` test before touching `m_d4`.

---

## Round 28 — order of work

Same rule as round 27, which worked: **every item ends in a compiled object and
a number.** If you run out of time, report fewer functions, not more proposals.

1. **`calc`** — from `calc_v1_decl_order`, eliminate the `f29` save. Report the
   **saved-register set and frame** for every variant, not just the word count
   and diff count. That is the scoreboard for this function now.
2. **`fn_80080E40` (−4)** — the missing content behind the correct deletion.
3. **`revisePos` (72/72)** — still never attempted, three rounds on the list, and
   you have the target read order written down: `0x9C`, `0xB4`, `0xB0`, `0x98`,
   `0xAC`, `0x94`. One compile.
4. **`fn_8007FFA0` (−8)** and **`addDokanMoveDiff` (−7)** — missing content, per
   your own diagnosis. Write the stores and lifetimes, compile.
5. **`fn_80080900` (−48)** — one pass building the objects your liveness table
   describes. Compile it whatever the frame comes out as, and report the frame
   and `_savegpr` level of the attempt.

`fn_80080670` (−3) last, only if time remains. Note its shape is the same family
as `calc`: your draft saves FPRs the target does not.

Work in `scratch/round28/`. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `GEMINI_*`, `CODEX_HANDOFF.md`, or
`HANDOFF.md`. **Do not run `ninja`, `configure.py`, `progress.py` or `land.py`**
— the tree is green, all five binaries byte-exact, and a concurrent build
destroys that.

---

## Reporting

Per function: target length, draft length from the object you built, **the
saved-register set** (which GPRs, which FPRs), `_savegpr` level, frame size,
then diff count.

The saved-register column is new and it is the important one. Diff count goes
last, and where frames differ, say so rather than treating the number as a
score.

**GAINED and LOST by name.** Five rounds accurate; keep it.

`poolcheck.py` via the CLI above, on the final object.
