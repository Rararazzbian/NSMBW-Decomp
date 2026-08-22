# Work order — round 24

**Read `AGENT_CONTEXT.md` first.** Two new sections came out of your round 23.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 23 — verified, and both results are now general rules

I reproduced everything independently before writing this.

**`set(sBgSetInfo)` is CLOSED.** 25/25, MATCHING, confirmed. And the lever is
recorded as general: *named temporaries in the target's READ order*, not merely
named temporaries. The three variants you measured are what make it a rule —
`mVec2_c` temps regressed, `f32` temps in natural order stayed at 7 diffs, and
only retail's `f4/f0/fC/f8` order closed it. That is a clean isolation.

**The `ProcMain` FPR negative is accepted, and your explanation upgraded my own
rule.** 45 lines reproduced exactly. You were right that the declaration-order
lever governs **callee-saved** `f28`..`f31` while `mMin`/`mMax` are living in
volatile `f0`..`f2`, which the scheduler allocates. I had stated the rule without
that boundary; `AGENT_CONTEXT.md` now carries it, with the diagnostic *check
which register file the residual is in before reaching for the lever*.

**`ProcMain` is closed as a bounded negative.** Do not open it again.

**31/8/0 on `d_bg_ctr` reproduces.** Both units clean on `poolcheck`.

---

## Round 23 — the half that did not land

The eight stubs are 906 words of target — the entire remaining value of the unit
— and the bodies you wrote for them are sitting in `.txt` files that have never
been through the compiler. **Prose that has not compiled is not a measurement.**
An un-compiled body is worth roughly nothing: every lever in this project is a
codegen lever, and none of them can be evaluated without an object file.

You stopped for a good reason — missing declarations — and you named them
accurately. I checked all of them. Here is the resolution, so that reason is gone.

### What already exists — just include it

| you needed | it is here |
|---|---|
| `nw4r::math::CosFIdx`, `SinFIdx` | `include/lib/nw4r/math/math_triangular.h` |
| `cM::atan2s` | `include/game/cLib/c_math.hpp` |
| `mMtx_c::ZrotS`, `mMtx_c::multVecZero` | `include/game/mLib/m_mtx.hpp` |
| `dScStage_c::getLoopPosX` | `include/game/bases/d_s_stage.hpp` |
| `dBaseActor_c::getCenterPos` | `include/game/bases/d_base_actor.hpp` |
| `dBc_c::mpOwner`, `mpNoHitActor` | `include/game/bases/d_bc.hpp` (lines 156, 171) |
| `PSVECMag`, `PSMTXTrans`, `PSMTXConcat` | `include/lib/revolution/MTX/vec.h`, `mtx.h` |

`EGG::Math` is in `include/lib/egg/math/eggMath.h`.

### What genuinely does not exist — propose it, do not add it

Four declarations are missing from the tree, and you read all four correctly out
of the target, so I am confident in them:

    nw4r::math::Atan2Idx(f32, f32)                       // bl Atan2Idx__Q24nw4r4mathFff
    nw4r::math::IntersectionSegment3Sphere(...)
    nw4r::math::DistSqSegment3ToSegment3(...)
    the SEGMENT3 and SPHERE types both of those take

`include/lib/nw4r/math/math_geometry.h` has `AABB` and `IntersectionResult` but
neither segment type. `math_triangular.h` has `Atan2FIdx`/`Atan2Deg`/`Atan2Rad`
but not `Atan2Idx` — and the target calls it out of line, so it is a real
function, not an inline.

**These are shared headers. Do not edit `include/**`.** Write your proposed
declarations to `scratch/round24/proposed_nw4r_geometry.hpp` and shadow it
locally to unblock yourself. I will land it alone, before anything else, because
a shared-header change has to be verified by itself.

Derive the signatures from the mangled names rather than guessing:

    IntersectionSegment3Sphere__Q24nw4r4mathFPCQ34nw4r4math8SEGMENT3PCQ34nw4r4math6SPHEREPfPf
    DistSqSegment3ToSegment3__Q24nw4r4mathFPCQ34nw4r4math8SEGMENT3PCQ34nw4r4math8SEGMENT3PfPf

Both take two pointer-to-const arguments and two `f32*` out-parameters.
`IntersectionSegment3Sphere` returns `bool` in the retail ABI unless the use site
says otherwise — check how the return value is consumed and report which you
chose and why. For the struct layouts, use the target's own load offsets: every
field access in `fn_80080900` tells you the size and order of `SEGMENT3`.

---

## Round 24 — compile the bodies

Work in `scratch/round24/`. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `GEMINI_*`, `CODEX_HANDOFF.md`, or
`HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build in this checkout
destroys that.

**One instruction: get all eight bodies compiling, and report each one's real
diff count.** Largest first — `fn_80080900` (256w), `fn_80080670` (130w), `calc`
(125w), `fn_80080E40` (121w), `fn_8007FFA0` (115w), `addDokanMoveDiff` (87w),
`revisePos` (72w), `fn_80080880` (32w).

A body that compiles at 200 diffs is worth more than one that reads correctly and
has never been built, because the first can be iterated and the second cannot.
**Get to a number for all eight before you optimise any one of them.**

Two of them have a shape worth naming while you write:

- **`fn_80080880` (32w)** is the cheapest and needs no missing headers at all —
  it swaps `f1`/`f2` to order a min/max and indexes `lbl_802EFBC0`/`lbl_802EFBD0`
  by rotation bits. Do this one first as a warm-up and to prove the loop.
- **`revisePos` (72w)** calls `fn_8007FFA0` three times, so land `fn_8007FFA0`'s
  signature before writing it.

The two lookup tables (`lbl_802EFBC0`, `lbl_802EFBD0`, `lbl_802EFBE0`,
`lbl_802EFBF0`) are unnamed `.rodata` in retail. Read their contents out of
`original/wiimj2d.dol` and write them as file-scope `static const` arrays — and
note from `AGENT_CONTEXT.md` that an unnamed retail label against a named draft
symbol is a **text** diff with identical bytes, so score on the union, not text.

---

## Reporting

Per function: target length, draft length, `_savegpr` level and frame size, then
**diff count** — the number, for all eight, not "analysed".

Draft length first, as you have been doing.

**GAINED and LOST by name** against round 23, both units.

`poolcheck.py` before you report. You are about to write eight bodies full of
float constants, which is precisely the condition that produces a wrong constant.

The proposed nw4r header, with the reasoning for each signature.
