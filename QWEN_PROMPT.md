# Work order — round 26

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 25 — verified, exact again, third round running

I reran your `diff_ctr.py` against round 24 and round 25 in the same session:

    round24:  MATCHED: 31  DIFFER: 8  MISSING: 0
    round25:  MATCHED: 32  DIFFER: 7  MISSING: 0

`fn_80080880` closed. `GAINED: {fn_80080880}`, `LOST: {}` — correct. And every
word count in your table reproduces exactly:

    calc              138 -> 129  (-9)
    fn_8007FFA0       114 -> 107  (-7)
    addDokanMoveDiff   91 ->  80  (-11)

−27 words, as you said. **The `psq_l`/GQR3 prediction is confirmed and it is now
a general rule in `AGENT_CONTEXT.md`.** You also wrote "none of the three
closed" in your own summary rather than letting the −27 stand as the headline.
That is the third consecutive accurate report and it is why I can spend this
brief on your leads instead of your arithmetic.

---

## The swap over-corrected, and that is the most useful thing in your round

Look at what actually happened to two of the three:

    fn_8007FFA0        was 114 vs 115  (-1)   now 107 vs 115  (-8)
    addDokanMoveDiff   was  91 vs  87  (+4)   now  80 vs  87  (-7)

**Do not revert this.** The swap is right — it emits the instruction retail
emits. What it revealed is that both functions were **missing real content all
along**, and the bulkier hand-written trig sequence was padding the length so
the totals looked close. You have traded two flattering numbers for two honest
ones, and honest ones are what you can act on.

So the question for both is no longer "why is my codegen different" — it is
**"what 7–8 words of behaviour am I not implementing?"** Read the target bodies
against your drafts for missing work, not for register choices.

---

## You declared two functions unfixable. One of them contradicts your own round 23.

This is the main item of the round.

You wrote of `calc`: *"This is an FPR-numbering issue and is NOT source-
addressable."* The diff you describe is your draft saving **f28/f29** where
retail saves **f30/f31**.

**`f14`–`f31` are callee-saved on this ABI.** Every register in that diff —
f28, f29, f30, f31 — is non-volatile. And the project rule, which you yourself
established in round 23 and which is in `AGENT_CONTEXT.md` under *"The FPR
declaration-order lever governs CALLEE-SAVED registers only"*, is that the
declaration-order lever **applies exactly there**. There is also a commit in
this repo titled **"FP register permutations ARE source-addressable"**
(`6b5f366f`).

Your round 23 result was that the lever does *not* reach `f0`..`f13` because the
scheduler owns those. You have now applied that boundary to `f28`..`f31`, which
is the side of the boundary where the lever **does** work. **Re-open `calc`**
and try the levers that are proven on callee-saved FPRs:

- declaration order of the float locals (lever 11 / the round-23 result);
- **named temporaries in the target's READ order** — this is what closed
  `set(sBgSetInfo)` for you, and it is the closest analogue;
- lever 12 (evaluation order) and lever 13 (the read-side def-point), and note
  `AGENT_CONTEXT.md` records that these **compose** within one statement pair
  rather than being alternatives.

Apply the same re-examination to **`fn_80080E40`** (+3, which you also called
not source-addressable — check the register file first and say which it is).

"Not source-addressable" is a conclusion this project has overturned repeatedly.
**Before writing it again, name which register file the residual is in and which
specific levers you tried.** If it is volatile, that is a real bounded negative
and I want it recorded. If it is callee-saved, it is almost certainly reachable.

---

## `fn_80080900` — stop searching, start reading

Your own log: frame went `0xc0 → 0xd0 → 0xf0 → 0xd0`, `_savegpr` went
`_24 → _23 → _21 → _22`. That is a circle, and it is two rounds now. The target
needs `_savegpr_20` and `0x170`. **Guessing at forms that might raise register
pressure is not converging and will not.**

You already did the useful work and then walked past it. You identified what the
target keeps live:

    r27  loop index i over the 4 corners
    r28  pointer into the corner buffer on stack (0xF8)
    r29  pointer to edge segment start (0xEC)
    r30  pointer to edge segment end (0xE0)
    r31  flags / loop counter

Five simultaneously-live values across a call. **Work forward from that, not
backward from the frame size.** Concretely: read the target body and write down,
in order, every distinct value it holds across the two
`DistSqSegment3ToSegment3` / `IntersectionSegment3Sphere` calls, and what each
one is *for*. Then write source that genuinely needs those five things live at
once. `0xa0` of missing stack is a lot — that is not a register-allocation
accident, it is locals you have not declared.

If a round of that does not converge either, **report the value-liveness table
you built and stop.** That table is worth more to the next round than four more
frame sizes.

---

## Round 26 — order of work

Work in `scratch/round26/`. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `GEMINI_*`, `CODEX_HANDOFF.md`, or
`HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build destroys that.

1. **`calc` (+4)** — the callee-saved FPR levers above. Highest confidence, and
   it tests a rule the project already owns.
2. **`revisePos` (72/72)** — still untouched from round 25's list, still pure
   ordering, still the same read-order lever. One compile.
3. **`fn_8007FFA0` (−8) and `addDokanMoveDiff` (−7)** — find the missing
   content, per the over-correction section above.
4. **`fn_80080E40` (+3)** — name the register file, then decide.
5. **`fn_80080900` (−48)** — the liveness table. Timebox it.

`fn_80080670` (−3) last.

---

## Reporting

Per function: target length, draft length, `_savegpr` level and frame size, then
diff count.

**GAINED and LOST by name.** Three for three; keep it.

`poolcheck.py` before you report.

For anything you conclude is not source-addressable: **the register file, and
the list of levers you actually tried.** A bounded negative is valuable and I
will record it — but only if it is bounded.
