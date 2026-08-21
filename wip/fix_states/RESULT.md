# fix_states RESULT — the 8 `executeState_*` `getLineUnitNo` residuals

Scope per brief: `executeState_Right30Right/Right60Down/Right30Left/Left60Down/
Left30Right/Right60Up/Left60Up/Left30Left` — all 8 overshoot by 1-3 words
because the reused `getLineUnitNo` call argument (a `f32` local surviving the
call) was previously written twice in source to avoid a spill.

Worked in `wip/fix_states/` from a copy of `wip/line_mng_merge/d_line_mng.cpp`
+ `shadow_include`. Nothing under `source/`, `include/`, `syms.txt`,
`slices/*.json`, `wip/line_mng_shared/`, `wip/line_mng_merge/`, or any other
agent's directory was touched. Compiles clean, no warnings. Full-file tally is
unchanged at **100/182, 27.7% by bytes** (no regression anywhere else in the
unit).

## 1. `tally.py` output, verbatim

```
matched 100/182 functions   2115/7631 words = 27.7% BY BYTES

 1193w              LEN OK  __sinit_\d_line_mng_cpp
  549w             MISSING  fn_800C31C0
  128w              LEN OK  CalcAdjustPosY__10dLineMng_cFff
  121w  117w vs 121w  STRUCTURAL  line_cross_chk1__10dLineMng_cFffRC7mVec2_c7mVec2_c7mVec2_cR7mVec2_
  104w              LEN OK  executeState_Right60Down__10dLineMng_cFv
  104w              LEN OK  executeState_Right30Right__10dLineMng_cFv
  103w  102w vs 103w  STRUCTURAL  executeState_Right30Left__10dLineMng_cFv
  102w              LEN OK  executeState_Left60Down__10dLineMng_cFv
  101w  100w vs 101w  STRUCTURAL  executeState_Right60Up__10dLineMng_cFv
  101w  100w vs 101w  STRUCTURAL  executeState_Left60Up__10dLineMng_cFv
  100w  99w vs 100w  STRUCTURAL  line_cross_chk2__10dLineMng_cFfRC7mVec2_c7mVec2_c7mVec2_cRf
  100w              LEN OK  executeState_Left30Right__10dLineMng_cFv
   99w  98w vs 99w  STRUCTURAL  executeState_Left30Left__10dLineMng_cFv
   94w  95w vs 94w  STRUCTURAL  start_line_move__10dLineMng_cFv
   79w              LEN OK  move_on_circle2__10dLineMng_cFff
   78w              LEN OK  lineRHUL_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   78w              LEN OK  circle_ul2_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   77w              LEN OK  move_on_circle4__10dLineMng_cFff
   76w              LEN OK  lineRHUR_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   76w              LEN OK  circle_ur2_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   75w              LEN OK  move_on_circle_speedset__10dLineMng_cFff
   75w              LEN OK  lineRHLL_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   75w              LEN OK  circle_dl2_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   74w              LEN OK  move_on_circle3__10dLineMng_cFff
   73w  72w vs 73w  STRUCTURAL  check_term__10dLineMng_cFv
```

**No new byte-exact matches** (still 100/182). But the LENGTH and CONTENT of
all 8 are substantially closer to target than the shipped baseline — see the
table below. `line_cross_chk1/2`, `check_term`, and `start_line_move` are
other authors' pre-existing residuals, untouched by me except `start_line_move`
which I read (see section 4) but did not edit.

## 2. Per-function length-and-match table

Length is TARGET/OLD-BASELINE/NEW-DRAFT. Diff-count is `harness.diff_fn`'s
raw instruction-line count (size line + differing rows, capped at 40 +
truncation marker) — **not comparable across different lengths**, included
only to show the SHAPE of the improvement.

| function | target | old (shipped) | old diff | new (this round) | new diff | new: LEN OK? |
|---|---:|---:|---:|---:|---:|---|
| executeState_Right30Right | 104 | 107 (+3) | 55 | **104 (±0)** | 8 | YES |
| executeState_Right60Down  | 104 | 106 (+2) | 61 | **104 (±0)** | 25 | YES |
| executeState_Right30Left  | 103 | 106 (+3) | 68 | 102 (-1) | 41 | no |
| executeState_Left60Down   | 102 | 104 (+2) | 60 | **102 (±0)** | 24 | YES |
| executeState_Left30Right  | 100 | 102 (+2) | 57 | **100 (±0)** | 9 | YES |
| executeState_Right60Up    | 101 | 102 (+1) | 63 | 100 (-1) | 41 | no |
| executeState_Left60Up     | 101 | 102 (+1) | 81 | 100 (-1) | 41 | no |
| executeState_Left30Left   | 99  | 100 (+1) | 58 | 98 (-1) | 41 | no |

Four of the eight (`Right30Right`, `Right60Down`, `Left60Down`, `Left30Right`)
are now **length-exact** and the remaining diff on each is 8-25 lines, not
55-81. The other four overshoot the OTHER direction now (1 word short instead
of 1-3 over) with a 41-line diff, because those four are the cases where the
`elseif` CONDITION itself already computes the identical offset expression
(see section 3) — MWCC's register allocator in this TU reuses that live value
instead of reloading it fresh, which target always does at this exact call
site (section 4).

None of the 8 are byte-exact. This is a genuine, reported negative on the
"close it all the way" goal, alongside a real, measured improvement on the
"how close, and why" question.

## 3. The source shape that helps: a class/struct-typed local, not a scalar

**Confirmed, re-tested in the full merged TU** (not just in isolation, per the
brief's NEW FACT about whole-TU-scoped decisions): a **plain scalar** local
still spills to a nonvolatile FPR here, exactly as author_states found in
isolation:

```cpp
f32 newX = mUnitBasePos.x + 16.0f;              // SPILLS: f31 nonvolatile,
u32 lineUnitNo = getLineUnitNo(newX, mUnitBasePos.y);  // frame 0x20 -> 0x30
```
Measured on `executeState_Left30Left`: `stwu -0x30`, `stfd f31,0x20(r1)` +
`psq_st f31,0x28(r1),0,qr0` in the prologue (both a double-store AND a
paired-single store for the one register — Gekko's paired-FPU save
convention, confirmed by direct disassembly). Also tried and also spills:
- `const f32 newX = ...;` (same spill)
- two plain scalars `f32 newX, newY` together (same spill)

**A `mVec2_c`-typed local does NOT spill.** Same call, same reuse, wrapped in
the class the field itself is (`mUnitBasePos` is already an `mVec2_c`):

```cpp
mVec2_c newBase(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
if (lineUnitNo == 8) {
    mUnitBasePos.x = newBase.x;
    ...
```
Frame stays `0x20` (target's frame), and the call arguments are staged
through the **outgoing-parameter stack slots** `0x8(r1)`/`0xc(r1)` and
reloaded from there after the call — **exactly target's own mechanism**
(confirmed byte-for-byte: target does `stfs f2,0xc(r1)` / `stfs f1,0x8(r1)`
before the call, `lfs f0,0x8(r1)` after, once per branch). This is a better
match to reality than the shipped "write it twice" form, which recomputes
from scratch after the call (extra `lfs`/`lfs`/`fadds`) and never touches the
stack slots at all.

Also tried, all giving the **same stack-slot mechanism** (no spill) as the
2-arg constructor, i.e. the "typed local" property is what matters, not the
exact spelling:
- `mVec2_c newBase; newBase.set(x_expr, y_expr);` (via `mVec2_POD_c`, `set()`)
- `mVec2_c newBase; newBase.y = ...; newBase.x = ...;` (field-by-field, either
  order)
- `mVec2_c newBase(mUnitBasePos); newBase.x = mUnitBasePos.x + 16.0f;` (copy +
  overwrite)
- `mVec2_c newBase(mUnitBasePos); newBase.incX(16.0f);` (copy + in-class
  increment — this one alone came out 1 word OVER instead of under, see below)

**This is the "third form."** It is strictly better than both prior options:
unlike the named scalar, it does not spill to a nonvolatile register (no
frame growth, no wrong bytes); unlike the "write it twice" form, it uses
target's actual outgoing-argument-slot reuse mechanism, not a full
recomputation.

## 4. Why none of the 8 close all the way: two separate, independent gaps

Reading target's actual bytes for `executeState_Left30Left` (see full
extraction below) shows the outgoing-argument-slot mechanism is real, but
target does two more things my draft does not, and they pull in OPPOSITE
directions:

**Gap A — target reloads the base coordinate again, even though it's already
live in a register.** Right before staging the call args, target does a
**redundant** `lfs f0, 0x50(r30)` (reloading `mUnitBasePos.x`) even on
functions where that exact value was *just* computed one basic block earlier
for the `elseif` condition itself and is still sitting in a live register.
MWCC in this TU does not carry that register value across the branch; it
reloads. My natural compile of the class-typed local DOES carry it across
(reuses the live register) — which is *cheaper*, correct-looking C++, but 1
word short of target.

Confirmed by forcing the reload with a diagnostic (not proposed as real
source) `*(volatile f32 *)&mUnitBasePos.x` read: this reproduces target's
exact reload+recompute sequence, byte for byte, for the whole rest of
`executeState_Left30Left` — see section 5. So Gap A **is** closeable by
source shape; I just could not find an idiomatic (non-`volatile`) C++
expression that reliably forces it. Every phrasing I tried that computes
`mUnitBasePos.x + 16.0f` from the *field* directly reused the live register.
This did not need forcing on the four functions whose `elseif` **condition**
does not already compute the exact reused expression (e.g. `Left30Right`'s
condition is bare `mPos.x < mUnitBasePos.x`, no `+16.0f`/`-16.0f` in it) —
those four naturally land within the 8-25 diff-line range in the table above,
because there's nothing for the allocator to reuse in the first place.

**Gap B — target never sets up `r3` (`this`) for the `getLineUnitNo` call at
all**, on every one of its 19 call sites file-wide (checked directly in
`wip/line_mng_shared/target.txt`, including sites preceded by an r3-clobbering
call, e.g. `start_line_move`'s `bl fmod` before its own `getLineUnitNo`
call). `getLineUnitNo(f32,f32)`'s body never reads `this` (it only forwards
`x,y` to two `static` calls), so this looks like MWCC eliding a genuinely
dead implicit-`this` parameter at the call site — but my draft's compiler
**always** emits `mr r3, r30` there regardless of source shape, including in
`start_line_move`, which is **author_core's own unmodified code, not touched
by me or by this round at all** (confirmed: `94w vs 95w draft` is the exact
number already in `MERGE.md`'s baseline, before any of my edits). This proves
Gap B is **not sourced from anything in the `executeState_*` family's body**
— it is a property of this merged TU's compile as a whole (consistent with
the brief's "whole-TU decision" warning about `_savegpr`/`_restgpr`; this
looks like the same class of phenomenon applied to a different MWCC
heuristic — likely resolved once the real `dBc_c`/`d_bc.hpp` and/or
`fn_800C31C0` land in the TU instead of the stub, though I did not get a
positive result testing that — see section 6).

**Gaps A and B are the same size (1 word) and cancel in opposite directions**,
which is why my natural (un-forced) draft comes out exactly 1 word SHORT
on the four functions where Gap A applies, while forcing Gap A closed with
`volatile` (section 5) comes out exactly 1 word LONG, purely from Gap B's
extra `mr`. Since Gap B is provably not fixable by editing these 8 functions,
**1 word is the closest this family can get from pure per-function source
work**, on the four where Gap A also applies. The other four (already
length-exact) are only missing Gap B's single `mr r3, r30` — see the diff
below.

## 5. Diagnostic evidence for Gap A (not shipped as real source)

`executeState_Left30Left` with the argument-forcing read:
```cpp
mVec2_c newBase(*(volatile f32 *)&mUnitBasePos.x + 16.0f, mUnitBasePos.y);
```
compiles to **100** words (target 99) and the diff against target is **purely
the one extra `mr r3, r30`** — every other instruction, operand, and branch
target for the rest of the function (30+ instructions) lines up exactly once
you account for that single one-word shift. This is the cleanest possible
demonstration that Gap A and Gap B are the whole story and nothing else is
wrong in this family. I did not ship this: `volatile` is a forcing trick, not
a claim about the real source, and per the reporting rules a guess must be
labelled a guess — this one is labelled and not merged.

## 6. Also tried and ruled out for Gap B (all still emit `mr r3, r30`)

- Giving `dBc_c::getUnitType`/`getUnitKind` (currently forward-declared only,
  see the file's own `MERGE NOTE`) trivial in-class bodies (`return 0;`), to
  test whether IPA needs to see *into* them to prove `this` unused by
  `getLineUnitNo`. No change — still emits the `mr`. (Reverted; not shipped,
  since it would also emit two unwanted extra symbols into the object.)
- Every one of the "third form" struct-typed-local variants in section 3.
- The original "write it twice" form (also has the `mr`, confirmed already
  present in the shipped baseline — this is not a regression I introduced).

I did not find a source-local fix for Gap B. It is reported as a whole-TU
property, not a per-function one, consistent with the brief's warning about
this exact class of MWCC decision.

## 7. Deliverable: the bodies (mergeable block)

All 8 bodies as currently in `wip/fix_states/d_line_mng.cpp` (lines
1433-1691+), unchanged from what's in the file — copy the corresponding
`executeState_*` definitions directly. Representative examples (the other 6
follow the identical pattern, `x`/`y` and `+`/`-` swapped per family, and
`mUnitBasePos = newBase` vs `mUnitBasePos.x = newBase.x` matching the
per-branch axis-update pattern already verified in author_states's RESULT.md):

```cpp
void dLineMng_c::executeState_Left30Left() {
    mVec2_c old = mPos;
    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 16.0f) + 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = 0.5f * mSpeed.x;
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mVec2_c newBase(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 8) {
            mUnitBasePos.x = newBase.x;
            mStateMgr.changeState(StateID_Left30Right);
        } else if (lineUnitNo == 0xA) {
            mUnitBasePos.x = newBase.x;
            mStateMgr.changeState(StateID_Right30Right);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    }
}

void dLineMng_c::executeState_Right30Left() {
    mVec2_c old = mPos;
    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = -(0.5f * mSpeed.x);
    mPos.x += mSpeed.x;
    mPos.y = mUnitBasePos.y - 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = -(0.5f * mSpeed.x);
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftupper(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mVec2_c newBase(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 8) {
            mUnitBasePos = newBase;
            mStateMgr.changeState(StateID_Left30Right);
        } else if (lineUnitNo == 0xA) {
            mUnitBasePos = newBase;
            mStateMgr.changeState(StateID_Right30Right);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    }
}
```

Full text for all 8 (plus their unchanged `initializeState_*`/`finalizeState_*`
siblings, not touched this round) is in `wip/fix_states/d_line_mng.cpp`
between the `executeState_Left30Left` and `executeState_Right60Up`
definitions.

## 8. Recommendation

Do not land this round's bodies over the shipped baseline as-is: none are
byte-exact, and swapping "known +1..+3 over, wrong content past the call" for
"known ±0..1, wrong by exactly one `mr r3,r30` or one missing reload" is a
lateral improvement in understanding, not a landing-ready fix. What IS
useful: the `mVec2_c`-typed-local shape (section 3) is strictly better than
what's shipped and should be the starting point for whoever next attacks Gap
B — once that whole-TU `mr r3, r30` elision is understood (likely tied to
`fn_800C31C0` or the real `d_bc.hpp` landing), these 8 functions plus
`start_line_move` (9 functions, ~900 words) may close in one pass without
further per-function work, since section 5 shows the remaining diff is
*purely* that one instruction once Gap A is also closed.
