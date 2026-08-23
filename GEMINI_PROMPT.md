# Work order — round 28

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## The landing assessment is first-rate, and I verified it

I checked your retail addresses against the symbol maps. **Every one is exact:**

    __vt__18dEnTorideKokoopa_c    = .data:0x80314360  size 0x5E4   ✓
    __vt__20KokoopaSpFumiCheck_c  = .data:0x80315298  size 0x10    ✓
    __vt__21MugenComboFumiCheck_c = .data:0x803152A8  size 0x10    ✓
    getFumiRev__12FumiCcInfo_cFv  = .text:0x800B07B0  size 0x50    ✓

A clear **NO** verdict, four concrete blockers, and a seven-header audit with a
retail symbol behind every entry. That is exactly what I asked for and it is
worth more than another percentage point — it converts "98.8% matched" into a
finite list of things to fix. `setQuakeDead` not closing is fine; this was the
valuable half.

Your headline is also right again: 246 raw + 2 artifacts = 248/251, and you
checked the artifact pair by name.

---

## Your blocker #1 is a definition-order problem, and it is fixable this round

You found the real defect and then described it as a constraint rather than a
bug:

    retail:  __vt__20KokoopaSpFumiCheck_c   at 0x80315298  (END of .data,
             __vt__21MugenComboFumiCheck_c  at 0x803152A8   after the 49 state
                                                            ID tables)

    draft:   __vt__20KokoopaSpFumiCheck_c   at 0x80314350  (START of .data,
             __vt__21MugenComboFumiCheck_c  at 0x80314968   before/just after
                                                            the kokoopa vtable)

**MWCC emits a class's vtable in the order the class is defined in the
translation unit.** `AGENT_CONTEXT.md` carries this for functions —
*"Function DEFINITION ORDER is part of the object"* and *"interleave by
ADDRESS, not by logical grouping"* — and it governs vtables the same way. Your
two helper classes are defined near the top of the file, so their vtables land
at the top of `.data`.

**Move both class definitions to the bottom of the translation unit**, after
everything that produces the state ID tables, so their vtables emit last. Retail
puts them at the very end; match that. This should:

- put `__vt__20KokoopaSpFumiCheck_c` and `__vt__21MugenComboFumiCheck_c` at
  `0x80315298`/`0x803152A8`;
- let `g_padData` go to **zero**, which is your own conclusion — "at link time
  this TU requires zero pad";
- and very likely resolve blocker #2, the `0x80` state-ID displacement, since
  that gap exists because 16 bytes of vtable are sitting where they should not
  be.

**Verify by construction, not by arithmetic** — the rule that settled the pad
region. Move them, recompile, and report the actual `.data` addresses of both
vtables and of `__vt__18dEnTorideKokoopa_c`, plus what `g_padData` had to become.
If it does not go to zero, say what it became.

---

## Completeness

All three items below are in scope. If a task's details seem incomplete, that is
not permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Round 28 — order of work

1. **Vtable placement.** Move the two helper class definitions to the end of the
   TU. Report the resulting `.data` addresses and the required pad size. This is
   blockers #1 and #2, and it is the difference between a unit that can land and
   one that cannot.
2. **`setQuakeDead` (352/340, 84 diffs).** Your diagnosis stands: the draft
   hoists a `0` across `UnKnownScoreSet` into non-volatile `r29` and grows the
   frame to `0x40`, where the target tests `searchBaseByID`'s `r3` directly and
   persists nothing. Stop the constant living across the call.
3. **Re-verify `__sinit` after step 1.** Moving 16 bytes of `.data` is exactly
   the class of change that broke it in round 25. Check it explicitly and report
   its diff count whatever it is.

`initializeState_Jump` / `BigJump` are closed as bounded negatives. Do not
reopen them.

Continue in `scratch/gemini_round24/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`** — and still do not attempt the landing itself. I
will do the header promotion and the five-binary verification.

---

## Reporting

- Baseline **248/251, 30,816 bytes**. Compute the headline once.
- **GAINED and LOST by name**, including the artifact-matched pair by name.
- **The `.data` address of every vtable in your object**, before and after the
  move, against the retail addresses above.
- **`__sinit`'s diff count**, explicitly.
- `poolcheck.py` output.
- An updated landing verdict: which of your four blockers are now cleared.
