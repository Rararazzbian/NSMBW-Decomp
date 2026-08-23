# Work order — round 31

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 30 verified. And I have to correct the landing model — including my own.

Your manifest answers are accepted. `d_actor_manager.hpp` keeps its promotion on
the `deadProc` evidence; `setKind` is dropped from `d_cc.hpp`. Both were direct
answers with the evidence attached, which is what I needed to act.

Fifteen ternary variants, all shapes reported, no closure claimed. The
`(fBase_c *)mUnk770` result is the useful one and you read it correctly: MWCC
sees `r3` already holds the value and elides the `li r3, 0`, giving 84 words
against 85 with the right frame and the right register set. That is a real
finding and it narrows the question to one sentence — *how do I get MWCC to
emit a redundant `li r3, 0` on the null path?*

## The correction: `initializeState_Jump` and `BigJump` are NOT closed

I read `tools/auto_decomp/land.py`. The gate is:

    ninja  &&  python progress.py --verify-bin   ->  5/5 binaries hash-identical

and a slice is a **contiguous address range per section**. There is no mechanism
for landing a unit with holes in it. **248/251 cannot land. 251/251 can.** Every
unmatched function in this TU is a landing blocker, and that includes the two
you and I have both been calling bounded negatives for several rounds.

I was wrong to accept that framing, and I have been repeating "do not reopen
them" in three consecutive briefs. Reopen them. They are now the priority,
because I have taken them apart and they are not bounded at all.

## What I found in them — and they are twins, so one fix closes both

    target : 90 words / frame 0x30 / GPR [30,31] / FPR none
    draft  : 90 words / frame 0x30 / GPR [30,31] / FPR none

Words, frame and both save sets already agree. Six diffs, every one of them a
register number. This is the `revisePos` situation exactly, and `revisePos`
closed.

The whole difference lives in the last four statements, which are identical in
both functions:

    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    mSpeed.x = (muki * rate) * speed.x;

**I got it from 6 to 5.** Moving the `speed.x` read to its own local declared
after the `mSpeed.y` store fixes three of the six:

    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;

What remains is one question, and here is the complete map of it. Retail's
allocation order in this region is:

    f0 = speed.y      f1 = rate      f2 = speed.x      f3, f4 = the int->float pair

The draft's is:

    f0 = speed.y      f1 = rate      f2, f3 = the int->float pair      f4 = speed.x

`(float)l_EnMuki[mDirection]` lowers to the magic-constant idiom — store the int
to the stack, `lfd` it, `lfd` the constant, `fsubs`. That consumes two FP
registers. **Retail allocates `speed.x` before those two. The draft allocates it
after**, and that single inversion produces all five remaining rows:

    66  T: lfd   f4, "@75355"@sda21(r0)   D: lfd   f3, "@27448"@sda21(r0)
    68  T: lfs   f2, 0x10(r1)             D: lfs   f4, 0x10(r1)
    69  T: lfd   f3, 0x18(r1)             D: lfd   f2, 0x18(r1)
    71  T: fsubs f0, f3, f4               D: fsubs f0, f2, f3
    73  T: fmuls f0, f0, f2               D: fmuls f0, f4, f0

Note row 73: retail keeps the source's operand order, `(muki*rate) * speed.x`.
The draft commutes it, because in the draft `speed.x` is in the higher register.
Fix the allocation and the commute goes with it.

**Twenty-four source shapes are already ruled out.** Do not spend the round
re-deriving them:

- statement reordering of those four lines, in every permutation — floor of 5;
- `float sx = speed.x` declared before `rate`, between `rate` and `muki`, after
  `muki`, and after the `mSpeed.y` store — 5, 5, 7, 5;
- both components hoisted to locals — 7;
- the multiply commuted (`sx * (muki * rate)`) — 5;
- parentheses removed, `rate * muki` instead of `muki * rate` — 6;
- `muki` inlined into the expression instead of a local — 7 or 8;
- the cast split as `int mi = l_EnMuki[...]` then `(float)mi` — 7;
- `mVec2_c sp = speed;` copied first — 36, much worse;
- `const f32 *sp = &speed.x;` with `sp[0]`/`sp[1]` — 6. The cursor trick that
  closed `revisePos` does **not** work here.

The floor is 5 and statement order cannot break it. **This needs a different
form, not a different order** — that is the rule the `revisePos` close produced
and it is the one that applies. Things I have not tried and would try next:

- change where `speed` itself comes from — the two `mVec2_c` assignments in the
  `if (flag)` block above may be forcing the stack layout that fixes the load
  order. Try assigning `speed.x`/`speed.y` component-wise in those branches
  rather than copying the whole `mVec2_c`.
- change the type or source of `muki`. `l_EnMuki` is an `int` array; if the
  retail declaration were `s16`, `f32`, or a different array entirely, the cast
  idiom changes and so does its register cost. **Check what `l_EnMuki` actually
  is in the retail symbol map before assuming.**
- `calcJumpRate()` returns into `f1`. Whether `rate` is a local or inlined into
  the expression changes when `f1` frees.

---

## Completeness

All three items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Round 31 — order of work

### 1. `initializeState_Jump` and `initializeState_BigJump` — the priority

Start from the 5-diff form above, not from the committed source. One fix closes
both; verify both every time. **Two closures here take the unit to 250/251 and
leave a single function between it and landing.**

**Acceptance:** ≥5 new variants, none from the ruled-out list, each with the
`fndiff.py` count for *both* twins. Report the allocation order you observe
(which value lands in f2, f3, f4) for each — that is the diagnostic, not the
diff count.

### 2. `setQuakeDead` — one narrow question

Not another fifteen-variant sweep. The `(fBase_c *)mUnk770` variant has the
right frame and the right registers and is one word short because MWCC elides a
`li r3, 0` it can prove is redundant. So find a null that MWCC **cannot** prove
is already in `r3` and which does not CSE with the two `sth` zeros. Ideas worth
one build each: a null of a *different pointer type* than the ternary's result;
the condition tested on a different expression from the argument; the argument
passed through a local so the tested value and the passed value are distinct
names.

**Acceptance:** ≥3 variants, each reported with words / frame / non-volatile set
/ whether `li r3,0` + `b` into a shared `cmpwi` is present.

### 3. Landing readiness statement

Given the gate above, tell me plainly: **with the current object, is this unit
landable — yes or no, and what is the exact remaining list?** If any of the
three closes this round, fold it, rebuild, and run `fndiff.py --all` plus
`poolcheck.py`. I want one number I can act on.

---

## Constraints

Continue in `scratch/gemini_round24/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `tools/**`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`**, and do not attempt the landing. I do the header
promotion and the five-binary verification.

---

## Reporting

- Jump/BigJump variant table: file, source shape, both twins' diff counts, and
  the observed f2/f3/f4 allocation.
- `setQuakeDead`: ≥3 variants with the four columns.
- `fndiff.py --all` if anything folded, pasted whole.
- **GAINED and LOST by name**, including the artifact-matched pair by name.
- The landing readiness statement.
- `poolcheck.py` output.
- `NOT REACHED`, if anything.
