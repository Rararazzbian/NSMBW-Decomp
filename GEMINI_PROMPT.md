# Work order — round 25

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 24 is your best round on this project, and the numbers are yours

Verified independently against a fresh compile before writing this:

    GAINED: 12 functions, 2,412 bytes
    LOST:    0 functions,     0 bytes

**Your delta is exactly right — all three figures.** +12, +2,412, and zero lost.
`executeState_AttackSearch` (512 B) closed as predicted, and you took
`preExecute`, `moveRevise`, `calcAttackTarget`, `calcJumpRate` and
`checkDownJump` with it — the whole length-wrong group I flagged, in one round.

You are at **243/251 (96.8%)** with the naming-artifact rule applied, and this
is your third consecutive clean loss report.

One defect, and it is the same family as last round's: you wrote **243** and
then **"Under Union Gate: 245 (accounting for 2 naming artifacts)"**. Your 243
*already* includes those two. You counted the same adjustment twice. Last round
it was a table with 15 rows under a heading saying 16. **Derived figures are
where your reporting still slips — compute the headline once and do not adjust
it a second time.**

Everything else checks: byte total exact, poolcheck clean, LOST genuinely zero.

---

## Eight functions left. Three have never been written.

**1,528 bytes of real unmatched work**, and the cheapest 204 bytes of it is code
you have simply never attempted:

     80 B  getFumiRev__12FumiCcInfo_cFv                            UNWRITTEN
     64 B  __dt__21MugenComboFumiCheck_cFv                         UNWRITTEN
     60 B  operate__21MugenComboFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c  UNWRITTEN

These are two small helper classes in the TU, not `dEnTorideKokoopa_c` methods,
which is presumably why they fell off the list. A destructor and two small
methods. **Do these first** — they are the last unwritten code in the unit.

Then, in order:

     76 B  hitCallback_PenguinSlide   (76/76, ONE diff — your `r3`/`r4` read)
    360 B  initializeState_Jump       (360/360, 6 diffs)
    360 B  initializeState_BigJump    (360/360, 6 diffs)
    340 B  setQuakeDead               (352/340, 84 diffs)
    188 B  movelimitCheck             (196/188, 39 diffs)

On **Jump / BigJump (720 bytes together)**: you have carried these for three
rounds without ever answering the question I keep asking. **State which register
file the six diffs are in, in your report, before drawing any conclusion.** If
`f0`..`f13`, they are volatile, the lever does not apply, `AGENT_CONTEXT.md`
records that as a bounded negative, and you should say so and stop. If
`f14`..`f31`, they are callee-saved and the lever is proven — and note the
result from the other unit this week: **when your draft saves a callee-saved FPR
the target does not, that spurious save is the defect.** Compare the two
prologues directly and drive the saved-register sets to equality.

`setQuakeDead` (84 diffs) and `movelimitCheck` (39 diffs) are the last two
length-wrong functions. You closed five of these in round 24 — repeat exactly
whatever you did there.

---

## Round 25 — order of work

Continue in `scratch/gemini_round24/` or start `scratch/gemini_round25/` from it;
either is fine, but **do not re-derive anything.**

Keep round 24's two process habits — they worked, and one of them saved a round:
write source to disk after every closure, and append to `GEMINI_RESPONSE.md` as
you go rather than composing it at the end.

Do not touch `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`,
`configure.py`, `QWEN_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build destroys that.

**This unit can finish.** Eight functions, and three of them are unwritten
helpers. If the round goes well you will be arguing with me about landing it,
which is a conversation worth having — so keep every change landable and do not
introduce anything you would not put in `source/`.

---

## Reporting

Round 23/24 format. It works.

- Baseline is **243/251**, the figure above. **Compute your headline once.** Do
  not apply the naming-artifact adjustment twice.
- **GAINED and LOST by name.** Three clean rounds; keep it.
- Per function: draft size first, then target size, then status.
- **Jump/BigJump: the register file, explicitly.** Fourth time of asking.
- `poolcheck.py` output.
