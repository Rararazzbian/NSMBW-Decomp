# Work order — round 27

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 26: headline exact, regression repaired, and you answered the question

Verified independently. **Your figures are exactly right — all of them:**

    246/251 raw + 2 artifacts = 248/251 (98.80%)
    24,420 + 6,396 artifact bytes = 30,816 matched bytes
    LOST: 0

`__sinit` is back to 4 diffs. You recalibrated the pad against the new vtables
and recovered **5,784 bytes**, plus `hitCallback_PenguinSlide`. Net +5,860.

You also used the *corrected* baseline I gave you (24,956) instead of carrying
forward your own inflated 30,880, and you checked the artifact-matched pair
explicitly. Both were asked for and both were done. That is the derived-figure
defect closed.

**And you answered the Jump/BigJump question.** `f0`..`f4`, volatile, zero
callee-saved FPRs on either side — so the lever does not apply and you recorded
it as a bounded negative and stopped. That is exactly the right handling, and it
retires 720 bytes from the board as unreachable rather than leaving them to be
re-litigated every round. Fifth time of asking, and worth it.

---

## One function left, and then the real question

    340 B  setQuakeDead   (352/340, 84 diffs)  <- the last actionable function
    360 B  initializeState_Jump      bounded negative, volatile FPRs
    360 B  initializeState_BigJump   bounded negative, volatile FPRs

Your `setQuakeDead` diagnosis is concrete and I think it is right: the target
calls `searchBaseByID` and tests the returned `r3` directly without persisting
it, while your draft hoists a `0` across `UnKnownScoreSet` into non-volatile
`r29` and grows the frame to `0x40`. That is the same family as the result from
the other unit this week — **a value held across a call that the target does not
hold** — so the fix is to stop the constant living across the call: sink it to
its use, or restructure the ternary so nothing survives `UnKnownScoreSet`.

**That is item one and it should not take the whole round.**

---

## Item two: tell me whether this unit can actually land

This is now the more important question, and it is the one nobody has answered.

The unit is at 98.8% matched **in a scratch harness**. Matching is not landing.
`AGENT_CONTEXT.md` carries the rule that a high tally does not mean a unit is
landable, because **the tally never links**. Before anyone proposes putting this
in `source/`, I need a written landing assessment from you. Specifically:

1. **`g_padData`.** You have said it is a harness artifact that disappears in a
   real link because `d_enemy_state.o` supplies those bytes. **Is that still
   true after the recalibration?** You have now sized it against your own new
   vtables — which means it is currently compensating for something real. State
   plainly: at link time, with `d_enemy_state.o` present, does this TU need a
   pad at all? If it does, it cannot land as written.

2. **The shadowed headers in `scratch/gemini_round24/include/`.** List every
   header you are shadowing and every declaration you have added or changed in
   them. For each, say whether it is (a) a genuine correction that belongs in
   the real `include/`, or (b) a local hack that only works in the harness.
   Anything in category (b) blocks the landing.

3. **New symbols.** `MugenComboFumiCheck_c`, `KokoopaSpFumiCheck_c`,
   `FumiCcInfo_c` — their vtables are now real `.data` in your object. Do those
   vtables exist in retail at the addresses your layout implies? You have the
   `.data` dump technique from the round-23 work; use it.

4. **Anything else you would not put in `source/` as written.**

An honest "here are three things that block it" is worth far more than another
percentage point. **Do not attempt the landing yourself** — do not run `ninja`,
`configure.py`, `progress.py` or `land.py`, and do not touch `source/`,
`include/`, or `slices/`. I want the assessment, not the attempt.

---

## Round 27 — order of work

1. `setQuakeDead` — the value held across the call.
2. The landing assessment, items 1–4 above, in writing.

Continue in `scratch/gemini_round24/`. Keep writing source after every closure
and appending to the report as you go.

Do not touch `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`,
`configure.py`, `QWEN_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run
`ninja`, `configure.py`, `progress.py` or `land.py`.**

---

## Reporting

- Baseline **248/251, 30,816 bytes**. Compute the headline once.
- **GAINED and LOST by name**, including the artifact-matched pair by name.
- `poolcheck.py` output.
- **The landing assessment as a numbered section**, with a one-line verdict at
  the top: can this unit land as written, yes or no?
