# Work order — round 26

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 25: four real closures, and one 5,784-byte regression you did not see

The four gains are real and verified:

    + 188  movelimitCheck
    +  80  getFumiRev__12FumiCcInfo_cFv
    +  64  __dt__21MugenComboFumiCheck_cFv
    +  60  operate__21MugenComboFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c

All three unwritten helpers written, plus the last length-wrong function. Good.

**But `__sinit` has broken, and it is the largest function in the unit.**

    round 24:  __sinit  4 diffs   (naming artifact -- effectively matched)
    round 25:  __sinit  200 diffs (genuinely broken)

You reported **"LOST Functions: 0 (Zero regressions across the entire
translation unit)"** and **"Matched Bytes: 30,880"**. The real byte total is
**24,956**. You are over by 5,924 — almost exactly `__sinit`'s 5,784 bytes. The
round is a net **loss** of ~5,392 bytes, not a gain of 392.

### Why it broke, and why your own tool missed it

`__sinit` was never in your *raw* matched set — it sat at 4 diffs and counted as
matched only under the naming-artifact rule. So a diff-set comparison of raw
matches shows no loss. **That is the blind spot: a function can regress out of
"matched-by-rule" without ever leaving the raw matched set.** Check the
artifact-matched functions explicitly, every round. There are only two.

The cause is your own round-25 work, and it is not a mistake so much as a
consequence you did not follow through:

    2088:  virtual bool operate(int &result, dEn_c *en, FumiCcInfo_c &fumi);

`MugenComboFumiCheck_c` and `KokoopaSpFumiCheck_c` have **virtual** methods, so
they have **vtables**, and those vtables land in `.data` ahead of
`__vt__18dEnTorideKokoopa_c`. That displaces the layout your `g_padData[128]`
was calibrated to, and `__sinit` — which references the kokoopa vtable — comes
apart.

`g_padData` is still in your source and unchanged. It is not wrong; it is now
**mis-sized**, because there is real new data in front of the thing it was
padding to.

---

## Round 26 — order of work

1. **Recalibrate the pad and get `__sinit` back.** The new vtables are real data
   that belongs in this TU, so the answer is not to delete the classes. Work out
   how many bytes the two new vtables occupy, reduce `g_padData` by exactly that
   much, recompile, and confirm `__sinit` returns to 4 diffs. If the arithmetic
   does not land it, dump the `.data` layout of your object and compare it to
   retail's at `0x803142E0`–`0x80314360` — **read the bytes, do not reason about
   them**, which is the rule that settled this region last time.

   This is 5,784 bytes and it is the whole round if it needs to be.

2. **`hitCallback_PenguinSlide` — 76 B, ONE diff**, your own `r3`/`r4` read.
   Cheapest thing on the board.

3. **`initializeState_Jump` / `initializeState_BigJump` — 360 B each, 6 diffs.**
   **State which register file the diffs are in. Fifth time of asking, and this
   round I want the answer before any analysis of them.** If `f0`..`f13`, they
   are volatile, the lever does not apply, and that is a recorded bounded
   negative — say so and stop. If `f14`..`f31`, they are callee-saved and worth
   720 bytes.

   A result from the other unit this week that applies directly: **when your
   draft saves a callee-saved FPR the target does not, look for a value being
   held across a call that the target stores to memory before the call.** That
   was worth three spurious saves there. Compare the two prologues, then compare
   what crosses each `bl`.

4. **`setQuakeDead` (352/340, 84 diffs)** — the last length-wrong function. You
   closed six of these across rounds 24 and 25; same approach.

That is the unit. Five functions and the `__sinit` recalibration.

Continue in `scratch/gemini_round24/` or branch a round-26 directory from it.
Keep writing source to disk after every closure and appending to the report as
you go — both habits have now saved a round each.

Do not touch `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`,
`configure.py`, `QWEN_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run
`ninja`, `configure.py`, `progress.py` or `land.py`.**

---

## Reporting

- Baseline is **246/251 raw, 24,956 matched bytes** — the corrected figures
  above, with `__sinit` counted as broken. Do not carry forward 30,880.
- **Check both artifact-matched functions explicitly** (`__sinit`,
  `executeState_ShellAtk_St`) and state their diff counts. They are the two your
  raw comparison cannot see.
- **GAINED and LOST by name.** Your loss reporting has been clean for three
  rounds on raw matches; this round extend it to the artifact set.
- Compute the headline once. Do not apply the artifact adjustment twice, as in
  round 24.
- `poolcheck.py` output.
