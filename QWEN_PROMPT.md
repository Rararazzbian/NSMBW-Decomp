# Work order — round 23

**Read `AGENT_CONTEXT.md` first.** Three new sections went in from your round 22,
one of which records that I gave you a wrong fix and how you caught it.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 22 — you were right and I was wrong, on both halves

I told you to negate `y0` in the preamble so the loop would have a ready-made
negative operand. **You rejected it with evidence and you were correct twice
over:**

- The `fneg` I pointed at is not in `ProcMain` at all. It is at `0x8007E8A4`,
  inside `createObjList`. I read it out of the range and assumed the function.
- Pre-negating would have changed the **value**, not the shape: the loop computes
  `y0 - mY`, and `(-y0) + mY` is a different number. It was never a code-shape
  lever.

That is the second time you have caught an error in one of my briefs, and both
times by checking the premise rather than trying the instruction. Keep doing it.

**And you then found the real lever, which is now recorded as general:** MWCC
folds `a + (-b)` into `subf` when it is one expression, and **the two-statement
form survives the fold**:

    s32 ny = m_pObjList[i].mY;
    ny = -ny;
    pos.y = (f32)((int)((y0 + ny) << 4));      // neg, then add

Also kept: the coupling you found between the `pos.z = 0.0f;` store position and
the `add` — the early z-store was occupying the slot retail's `add` needed. That
kind of "an unrelated store was in the way" finding is worth reporting whenever
you see it.

**The `viewMin`/`viewMax` negative is accepted and closed.** A user-declared copy
constructor on `mVec3_c` forces the float path; you measured both alternatives,
you established that removing it regresses 160 functions, and you correctly
called it out of scope. That is recorded. **Do not revisit it.**

**`d_bg_ctr.cpp`: 30/39 on a first pass is a very good start.** And you did the
thing the prompt asked and most agents skip — you found that `prepare.py`
over-extended the range (the bundled split object spans several TUs), trimmed it,
and verified the end boundary by hand against `__ct__11dBgGlobal_cFv`. That is
exactly right.

---

## Round 23 — `d_bg_ctr.cpp`, the nine that differ

Work in your `d_bg_ctr` directory. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `GEMINI_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green and a concurrent build in this checkout would destroy that.

### 1. `ProcMain` — one last look, then stop

Two residual groups, both count-neutral:

- **28 preamble lines** — blocked on the copy constructor. **Out of scope, leave
  them.**
- **17 `mMin`/`mMax` lines** — pure FPR numbering, target `f2/f1/f0` against
  draft `f1/f0/f2`. You tried three shapes and all rotated the same way.

For the FPR group, one axis you have not reported trying: the **declaration
order of the two locals themselves**, independent of how they are built. The
rule is that callee-saved FPRs are handed out in DECLARATION order while the
schedule follows ASSIGNMENT order, and the way to decouple them is to declare
early and assign late (`mVec3_c mMin; mVec3_c mMax; ... mMin = ...;`). That is
the lever that closed `execute` for you in round 20. If it does not move the
numbering, **say so and stop** — write it up as a bounded negative with the
variants listed, and `ProcMain` is done as far as I am concerned.

### 2. `d_bg_ctr.cpp` — the six real functions

Nine differ; **`calc` (125w), `fn_8007FFA0` (115w), `revisePos` (72w),
`addDokanMoveDiff` (87w), `fn_80080670` (130w), `fn_80080880` (32w),
`fn_80080E40` (121w) and `fn_80080900` (256w) are all unwritten stubs** — draft
length 1. Those are authoring work, not matching work, and they are where the
unit's remaining bytes are.

Take them **largest first**: `fn_80080900` (256w), `fn_80080670` (130w),
`calc` (125w), `fn_80080E40` (121w), `fn_8007FFA0` (115w).

The one genuinely mismatched function is
`set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c` (25/25, 8 FPR-numbering
lines). Given three of its four siblings match, compare it against them — a
sibling that matches is the best available oracle for what shape the odd one out
should take. Note the warning in `AGENT_CONTEXT.md` though: **a mirror does not
necessarily take the mirrored fix**; two line-for-line mirrors once needed
opposite treatment. Measure, do not assume.

### 3. Run `poolcheck.py` before you report

    python tools/auto_decomp/poolcheck.py <draft.cpp> <shadow_include> <target.txt>

You were clean on both units last round. Keep it that way — and note that as you
start writing large new function bodies with float constants in them, this is
exactly when the wrong-constant class appears.

---

## Reporting

Per function: target length, draft length, `_savegpr` level and frame size, then
match status. **Draft length first** — a 1-word draft is a stub, not a mismatch,
and your round-22 table made that distinction clearly. Keep doing that.

**GAINED and LOST by name** against round 22, both units.

The two habits that have made your last three rounds productive: **measure the
variant you expect to fail**, and **check the premise before acting on it** —
including when the premise came from me.
