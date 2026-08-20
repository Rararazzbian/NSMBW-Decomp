# author_geom -- collision/intersection geometry family

Scope: the 16 functions listed in the brief (`line_cross_chk1/2/3`,
`line_cross_slope_check`, `line_cross_range_check`, `lineRHUL/UR/LL/LR`,
`circle_ul2/ur2/dl2/dr2`, `lineF_cross_chk`, `height_cross_chk`,
`width_cross_chk`). All 16 have full bodies below. **None are byte-exact
yet** -- 10 of 16 are exact-LENGTH matches against the target (correct
content, pure scheduling/register-allocation residual); the other 6 are 1-4
words short. Every number below is MEASURED off a real
`compile_draft`/`disasm`/`diff_fn` run against `wip/line_mng_shared/target.txt`,
not estimated.

Work is entirely in `wip/author_geom/`: `d_line_mng.cpp` (the draft) and
`local_shadow_include/game/bases/d_line_mng.hpp` (a corrected LOCAL copy of
the shared header, used only via `-i` ordering for this round's compiles --
`wip/line_mng_shared/` itself was never written to). The local header was
refreshed against the shared header's latest state (the `mov_to_*`/
`is_unit_circle*`/`getLineUnitNo` fixes from the parallel round) before this
report was finalized, so it carries no stale declarations.

## Header corrections this round is proposing (evidence, not applied to the shared file)

**Two structural findings, both proven from calling convention / caller
register behaviour, not analogy:**

1. **`line_cross_chk1`, `line_cross_chk2`, `line_cross_chk3`,
   `line_cross_slope_check`, `line_cross_range_check` are `static` member
   functions**, not plain instance methods as the shared header currently
   declares them. Proof: each one's GPR/FPR registers are *fully* consumed
   by its own declared parameter list with nothing left over for an implicit
   `this` -- e.g. `line_cross_chk3(f32, const mVec2_c&, const mVec2_c&)`
   uses exactly r3 (first ref), r4 (second ref), f1 (the float); `this`
   would have to occupy r3, but r3 is read as a plain vector (`lfs f2,
   0x0(r3)` / `lfs f0, 0x4(r3)`, i.e. `.x`/`.y`), never as a `dLineMng_c*`
   reaching `mDirVec`/`mSpeed`/etc. at those same offsets. Cross-checked
   against `circle_ul2_cross_chk` etc., which DO use `this` (r3 saved to
   r28/r30 and later dereferenced at `+0x50`, `+0x64`, `+0x6c` -- exactly
   `mUnitBasePos`, `mAngle`, `mStateMgr`) -- so the "no leftover register"
   test discriminates correctly on functions known to have `this` too.

2. **All five of the above, plus `height_cross_chk`, `width_cross_chk`,
   `lineF_cross_chk`, all four `circle_*2_cross_chk`, and all four
   `lineRH**_cross_chk`, return `bool`, not `void`.** Proven two ways, per
   this round's own directive to check caller behaviour before trusting
   "obviously a predicate":
   - *Internally*: every one of them sets `r3` to a live 0/1 on every
     control-flow path before `blr`, and several of them (`line_cross_chk1`,
     `line_cross_chk2`, `height_cross_chk`, `width_cross_chk`,
     `lineF_cross_chk`, all eight `circle_*2`/`lineRH**`) branch on their own
     callee's `r3` with `cmpwi r3,0x0` immediately after a `bl` to one of the
     others in this family -- that only makes sense if the callee returns a
     value.
   - *Externally*: I grepped every `bl` site into `height_cross_chk`,
     `width_cross_chk`, `lineF_cross_chk`, and all eight `circle_*2`/
     `lineRH**` functions across the **whole** `target.txt` (not just this
     unit's own internal calls). Every single caller (`line3h_cross_chk`,
     `line3v_cross_chk`, `line4_cross_chk`, `line5_cross_chk`, and the
     circle/RH dispatch chain around `0x800C3500`-`0x800C39E8`) does
     `cmpwi r3,0x0` / `mr r31,r3` right after the `bl` and propagates that
     value as its OWN return (`mr r3,r31` at its tail) -- never a clobber.
     This is the strongest evidence available and it is unanimous across
     every caller found, so it is reported as PROVEN, not inferred from the
     name.

`wip/author_geom/local_shadow_include/game/bases/d_line_mng.hpp` carries both
fixes (`static bool` for the five primitives, `bool` for the rest) with the
evidence inlined as comments, plus a **testing-only `friend` declaration**
for the free function below -- see next section.

## Bonus, out-of-scope dependency: `fn_800C1EE0`

`width_cross_chk`'s target body is a 5-argument setup ending in
`bl fn_800C1EE0` -- an address-named, symbol-less callee (one of MAPPING.md's
"Unnamed file-scope functions"). It is not one of the 16 assigned functions,
but `width_cross_chk` cannot be compiled or diffed without *some* callee
there, so I reconstructed and authored it too (see `d_line_mng.cpp`). Evidence
it is a **free `static` function**, not a class member: `width_cross_chk`
passes its OWN incoming `this` (r3) straight through to it unmodified (no
`mr r3, ...` anywhere in `width_cross_chk`'s body before the `bl`), and
`fn_800C1EE0` itself writes `this->mPos`/`this->mUnitBasePos` through that
same pointer, but it is called with **no mangled name at all** in the
target -- whereas every other `static` member in this unit
(`line_cross_chk1` etc.) DOES carry a real mangled name. A free function
needs `friend` access to touch `dLineMng_c`'s private `mPos`/`mUnitBasePos`,
so the local header adds one testing-only `friend` line for this round's
compiles. **This is not a proposed permanent header change** -- flagging it
for the lead to decide (maybe it should instead become a private static
member with a stripped symbol, if that's plausible for this build).
`width_cross_chk` itself is confirmed `bool` per the caller-check above, and
`fn_800C1EE0`'s own tail (falls straight through to `blr` without touching
`r3` after the `line_cross_chk1` call) is consistent with it also being
`bool`, propagated untouched.

## Results table (MEASURED)

Length first, per the project's own rule -- a differing count is meaningless
across mismatched lengths.

| function | target words | draft words | length match | differing words | byte-exact |
|---|---|---|---|---|---|
| `line_cross_slope_check` | 22 | 18 | NO (-4) | 20 | NO |
| `line_cross_range_check` | 19 | 19 | YES | 17 | NO |
| `line_cross_chk1` | 121 | 117 | NO (-4) | 75 | NO |
| `line_cross_chk2` | 100 | 99 | NO (-1) | 96 | NO |
| `line_cross_chk3` | 32 | 30 | NO (-2) | 17 | NO |
| `lineRHUL_cross_chk` | 78 | 78 | YES | 20 | NO |
| `lineRHUR_cross_chk` | 76 | 76 | YES | 18 | NO |
| `lineRHLL_cross_chk` | 75 | 75 | YES | 12 | NO |
| `lineRHLR_cross_chk` | 71 | 71 | YES | 16 | NO |
| `circle_ul2_cross_chk` | 78 | 78 | YES | 20 | NO |
| `circle_ur2_cross_chk` | 76 | 76 | YES | 18 | NO |
| `circle_dl2_cross_chk` | 75 | 75 | YES | 12 | NO |
| `circle_dr2_cross_chk` | 71 | 71 | YES | 16 | NO |
| `lineF_cross_chk` | 60 | 60 | YES | 10 | NO |
| `height_cross_chk` | 65 | 64 | NO (-1) | 38 | NO |
| `width_cross_chk` | 16 | 16 | YES | 6 (5 are the unnamed-callee's mangled vs bare name -- see below) | NO |

**Plainly: none of the 16 are byte-exact yet.** 10/16 are length-exact
(content is right, remaining diffs are register-allocation/instruction-
scheduling residuals, e.g. `f1`↔`f2`/`f3`↔`f4` swapped consistently, or a
branch-polarity choice cascading into shifted-but-parallel local-label
offsets -- inspected by hand, not just by word count). 6/16
(`line_cross_slope_check`, `line_cross_chk1/2/3`, `height_cross_chk`) are
short by 1-4 words and have real (not just cosmetic) structural differences
still open.

`width_cross_chk`'s 6 differing words are ALL on a single line: the target's
`bl fn_800C1EE0` (a bare, symbol-less callee) vs the draft's
`bl fn_800C1EE0__FP10dLineMng_cffRC7mVec2_cRC7mVec2_cRC7mVec2_cRC7mVec2_c`
(a real mangled name, since my `fn_800C1EE0` is an actual named C++ function
with a friend declaration). This is the expected, inherent limitation the
project's own docs describe for calls into unnamed statics compiled in
isolation -- everything else in `width_cross_chk` is byte-identical to the
target already. Not a defect to chase further at this grain; it should
resolve automatically once landed in the real TU alongside the real
(also-unnamed) `fn_800C1EE0`.

## What's proven vs what's still open, per function

- **`line_cross_chk3`**: logic is right (confirmed circle-crossing XOR test:
  true iff exactly one of `d(p2)`, `d(p3)` is inside the radius, or `d(p3)==0`
  exactly) and callers all confirm bool. Remaining gap: MWCC's flag-extraction
  idiom (`mfcr`/`extrwi`) vs the target's explicit branch-based `li r3,0x1`/
  `li r3,0x0` shape. Tried a `goto`-based rewrite (in the current draft) which
  closed 2 of the 4 missing words and restored the target's two-separate-
  return-point shape, but a register-naming swap (`f2`/`f4`) persists.
- **`line_cross_slope_check` / `line_cross_range_check`**: logic verified
  against every call site (both callers' stack layouts and branch conditions
  match exactly what these two are decoded to do). The target allocates a
  small stack frame (`stwu r1,-0x10`) and spills its two locals to it even on
  the early-return path; the current draft doesn't need a frame at all and is
  4/0 words short/equal respectively with materially different scheduling.
  Not resolved this round -- flagging as open rather than guessing further.
- **`line_cross_chk1`**: fully decoded (translate p4/p5 by `-p3`, slope-check,
  branch on `p1==slope` for the degenerate axis-aligned case, range-check,
  translate back). 4 words short; the diff is concentrated around the
  `p1-slope==0.0f` branch's exact instruction shape.
- **`line_cross_chk2`**: fully decoded, including the same-sign/opposite-sign
  x-coordinate pre-check before the slope-check. 1 word short.
- **`height_cross_chk`**: fully decoded, including the `mSpeed.y`/
  `mBaseSpeed`-sign-based conditional `change_dir()` call. 1 word short,
  concentrated in a `lfs`/`fadds` operand-order swap.
- **The 8 `circle_*2`/`lineRH**` functions**: this is where "solve the
  smallest completely, then substitute" paid off exactly as the brief
  predicted. All 8 share one template (translate by a per-corner offset,
  `line_cross_chk3` against a per-tier radius², `atan2s`, an angle-window
  clamp against a per-corner base offset, conditional `change_dir()` for the
  two "Up" corners only, `changeState` to the matching state). All 8 are now
  **length-exact**. The two nastiest details, both discovered by close
  reading of raw bytes rather than assumed by symmetry:
  - The "subtract 0x3f00"/"subtract 0x7f00" steps are NOT literal
    subtractions in source -- MWCC's `addis r3,r3,0x1` before the `subi`
    proves the real added constant is the *complement* (`0xc100` = `0x10000
    - 0x3f00`, `0x8100` = `0x10000 - 0x7f00`), which only needs the
    addis-then-subi split because `0xc100`/`0x8100` don't fit as a plain
    signed 16-bit `addi` immediate the way `0x100`/`0x4100` do. Read the
    literal DOL float pool to confirm the values used, per the project's
    "prove the pool pattern isn't just proving itself" rule: 16.0f, 8.0f,
    32.0f, 64.0f, 256.0f, 1024.0f, 0.1f, -0.1f, 0.0f were all read directly
    out of `original/wiimj2d.dol` (`.sdata2` `0x8042CB18`-`0x8042CB7C`), not
    assumed from context.
  - `circle_dl2`/`circle_dr2` and `lineRHLL`/`lineRHLR` (the "Down" corners)
    skip the `change_dir()` call that the "Up" corners (`circle_ul2`/
    `circle_ur2`, `lineRHUL`/`lineRHUR`) make -- confirmed by direct byte
    absence, not assumed from naming symmetry.
  - `circle_dr2`/`lineRHLR` specifically also want the early-return form
    (`return false;`) for the angle-out-of-range case while the other 6 want
    the shared-variable form (`result = false;`) -- tested both ways per
    function since the compiled shape differed (one grew a word, the other
    shrank one, under the same source template).

## Method notes (what actually moved the needle)

- **Size first, always.** Every fix in this round was applied, then measured
  against `target words` before looking at the differing-line count, per the
  project's rule. Two rounds of fixes actually made specific functions
  *worse* in length (a first attempt at the angle-clamp `else`-branch shape
  added a word to 6 of the 8 circle/RH functions while fixing the other 2) --
  caught immediately because length was checked every time, not just at the
  end.
- **p2/p3 evaluation order matters and is source-visible.** All 9 of the
  `(const mVec2_c&, mVec2_c, mVec2_c)`-shaped functions translate their
  THIRD parameter (`p3`, mapped to r6) before their SECOND (`p2`, mapped to
  r5) -- opposite of my first draft's order. Swapping fixed the r5/r6-swapped
  half of every one of these functions' diffs in one pass.
- **Float pool values were read from the DOL, not assumed.** Per the
  project's canonicaliser caveat (pool-symbol matching proves reference
  *pattern*, not value), every literal used above was independently
  extracted from `original/wiimj2d.dol`'s `.sdata2` bytes before being
  trusted.

## Full function bodies (self-contained; merge target: `wip/author_geom/d_line_mng.cpp`, lines 133-580)

```cpp
// ===========================================================================
// author_geom: collision/intersection geometry family
// ===========================================================================

bool dLineMng_c::line_cross_slope_check(const mVec2_c &a, const mVec2_c &b, f32 &slope, f32 &intercept) {
    f32 dx = b.x - a.x;
    f32 dy = b.y - a.y;
    if (dx == 0.0f) {
        return false;
    }
    slope = dy / dx;
    intercept = b.y - slope * b.x;
    return true;
}

bool dLineMng_c::line_cross_range_check(f32 a, f32 b, f32 v) {
    f32 lo, hi;
    if (b >= a) {
        lo = a;
        hi = b;
    } else {
        lo = b;
        hi = a;
    }
    return v >= lo - 0.1f && v <= hi + 0.1f;
}

bool dLineMng_c::line_cross_chk1(f32 p1, f32 p2, const mVec2_c &p3, mVec2_c p4, mVec2_c p5, mVec2_c &out) {
    p4.x -= p3.x;
    p4.y -= p3.y;
    p5.x -= p3.x;
    p5.y -= p3.y;

    f32 slope, intercept;
    if (line_cross_slope_check(p4, p5, slope, intercept)) {
        if (p1 - slope == 0.0f) {
            if (intercept != 0.0f) {
                return false;
            }
            out.x = p5.x;
            out.y = p5.y;
        } else {
            out.x = intercept / (p1 - slope);
            out.y = p1 * out.x;
        }

        if (!(out.x >= 0.1f && out.x <= p2 + 0.1f)) {
            return false;
        }
        if (!line_cross_range_check(p4.x, p5.x, out.x)) {
            return false;
        }
    } else {
        if (!(p5.x >= 0.0f && p5.x < p2)) {
            return false;
        }
        out.x = p5.x;
        out.y = p1 * p5.x;
        if (!line_cross_range_check(p4.y, p5.y, out.y)) {
            return false;
        }
    }

    out.x += p3.x;
    out.y += p3.y;
    return true;
}

bool dLineMng_c::line_cross_chk2(f32 p1, const mVec2_c &p3, mVec2_c p4, mVec2_c p5, f32 &out) {
    p4.x -= p3.x;
    p4.y -= p3.y;
    p5.x -= p3.x;
    p5.y -= p3.y;

    if (p4.x != 0.0f && p5.x != 0.0f) {
        if (p4.x >= 0.0f) {
            if (!(p5.x >= 0.0f)) {
                return false;
            }
        } else if (p5.x < 0.0f) {
            return false;
        }
    }

    f32 slope, intercept;
    if (line_cross_slope_check(p4, p5, slope, intercept)) {
        if (!(intercept >= -0.1f && intercept <= 0.1f + p1)) {
            return false;
        }
        if (!line_cross_range_check(p4.y, p5.y, intercept)) {
            return false;
        }
        out = intercept;
        return true;
    } else {
        if (p5.x != 0.0f) {
            return false;
        }
        if (!(p5.y >= 0.0f && p5.y < p1)) {
            return false;
        }
        out = p5.y;
        return true;
    }
}

bool dLineMng_c::line_cross_chk3(f32 p1, const mVec2_c &p2, const mVec2_c &p3) {
    f32 d3 = p3.x * p3.x + p3.y * p3.y - p1;
    if (d3 == 0.0f) {
        return true;
    }
    f32 d2 = p2.x * p2.x + p2.y * p2.y - p1;
    if (d3 < 0.0f) {
        if (d2 >= 0.0f) {
            goto ok;
        }
        return false;
    }
    if (d2 < 0.0f) {
        goto ok;
    }
    return false;
ok:
    return true;
}

// @unofficial TESTING-ONLY: unnamed file-scope helper (target `fn_800C1EE0`,
// per MAPPING.md's "Unnamed file-scope functions" list). Reconstructed from
// width_cross_chk's caller side and its own body: takes a dLineMng_c* that
// passes straight through from the caller's own `this` (unmodified before
// the call), so it is a free `static` function, not a class member (member
// statics in this unit -- line_cross_chk1 etc. -- carry real mangled names
// in the target; this one has none at all). NOT part of this round's
// assignment; authored only because width_cross_chk cannot be compiled or
// tested without it. See RESULT.md for the friend-declaration caveat.
bool fn_800C1EE0(dLineMng_c *pThis, f32 a, f32 b, const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3, const mVec2_c &origin) {
    mVec2_c out;
    // chk1 needs p2/p3 BY VALUE (it translates them in place); this helper
    // receives them by reference from its caller and makes its OWN local
    // copies here -- confirmed from the target: width_cross_chk passes its
    // own r5/r6 straight through unmodified (no copy at its call site), and
    // fn_800C1EE0's own body stores p2/p3 into ITS OWN stack slots before
    // calling line_cross_chk1.
    mVec2_c p2c = p2;
    mVec2_c p3c = p3;
    bool result = dLineMng_c::line_cross_chk1(a, b, origin, p2c, p3c, out);
    if (result) {
        pThis->mPos.x = out.x;
        pThis->mPos.y = out.y;
        pThis->mUnitBasePos.x = p1.x;
        pThis->mUnitBasePos.y = p1.y;
    }
    return result;
}

bool dLineMng_c::height_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c pt(p1.x + 16.0f, p1.y - 16.0f);
    f32 outY;
    bool result = line_cross_chk2(16.0f, pt, p2, p3, outY);
    if (result) {
        mPos.x = pt.x;
        mPos.y = outY + pt.y;
        mUnitBasePos.x = p1.x;
        mUnitBasePos.y = p1.y;
        if (mSpeed.y < 0.0f) {
            if (mBaseSpeed > 0.0f) {
                change_dir();
            }
        } else {
            if (mBaseSpeed < 0.0f) {
                change_dir();
            }
        }
    }
    return result;
}

bool dLineMng_c::width_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin;
    origin.y = p1.y - 16.0f;
    origin.x = p1.x;
    return fn_800C1EE0(this, 0.0f, 16.0f, p1, p2, p3, origin);
}

bool dLineMng_c::lineF_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 8.0f;
    origin.x = p1.x + 8.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(64.0f, p2, p3);
    if (result) {
        mAngle = cM::atan2s(p3.y, p3.x);
        mUnitBasePos.x = p1.x;
        mUnitBasePos.y = p1.y;
        mStateMgr.changeState(StateID_Circle);
    }
    return result;
}

bool dLineMng_c::circle_ul2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 16.0f;
    origin.x = p1.x + 16.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(256.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0xc100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle += 0x3f00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            change_dir();
            mStateMgr.changeState(StateID_Circle2x2Leftup);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::circle_ur2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 16.0f;
    origin.x = p1.x;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(256.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle -= 0x100;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            change_dir();
            mStateMgr.changeState(StateID_Circle2x2Rightup);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::circle_dl2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y;
    origin.x = p1.x + 16.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(256.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x8100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle += 0x7f00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle2x2LeftDown);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::circle_dr2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    p3.x -= p1.x;
    p3.y -= p1.y;
    p2.x -= p1.x;
    p2.y -= p1.y;

    bool result = line_cross_chk3(256.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x4100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle -= 0x4100;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle2x2RightDown);
        } else {
            return false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHUR_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 32.0f;
    origin.x = p1.x;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(1024.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle -= 0x100;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            change_dir();
            mStateMgr.changeState(StateID_Circle4x4Rightup);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHUL_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 32.0f;
    origin.x = p1.x + 32.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(1024.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0xc100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle += 0x3f00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            change_dir();
            mStateMgr.changeState(StateID_Circle4x4LeftUp);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHLL_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y;
    origin.x = p1.x + 32.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(1024.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x8100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle += 0x7f00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle4x4LeftDown);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHLR_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    p3.x -= p1.x;
    p3.y -= p1.y;
    p2.x -= p1.x;
    p2.y -= p1.y;

    bool result = line_cross_chk3(1024.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x4100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle -= 0x4100;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle4x4RightDown);
        } else {
            return false;
        }
    }
    return result;
}
```

## Header change needed to merge this (evidence above, not applied to the shared file)

In `game/bases/d_line_mng.hpp`, change:
```cpp
void line_cross_slope_check(const mVec2_c &, const mVec2_c &, f32 &, f32 &);
void line_cross_range_check(f32, f32, f32);
void line_cross_chk1(f32, f32, const mVec2_c &, mVec2_c, mVec2_c, mVec2_c &);
void line_cross_chk2(f32, const mVec2_c &, mVec2_c, mVec2_c, f32 &);
void line_cross_chk3(f32, const mVec2_c &, const mVec2_c &);
...
void height_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
void width_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
...
void lineF_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
void circle_ul2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
void circle_ur2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
void circle_dl2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
void circle_dr2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
void lineRHUR_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
void lineRHUL_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
void lineRHLL_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
void lineRHLR_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
```
to:
```cpp
static bool line_cross_slope_check(const mVec2_c &, const mVec2_c &, f32 &, f32 &);
static bool line_cross_range_check(f32, f32, f32);
static bool line_cross_chk1(f32, f32, const mVec2_c &, mVec2_c, mVec2_c, mVec2_c &);
static bool line_cross_chk2(f32, const mVec2_c &, mVec2_c, mVec2_c, f32 &);
static bool line_cross_chk3(f32, const mVec2_c &, const mVec2_c &);
...
bool height_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool width_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
...
bool lineF_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
bool circle_ul2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
bool circle_ur2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
bool circle_dl2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
bool circle_dr2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
bool lineRHUR_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
bool lineRHUL_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
bool lineRHLL_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
bool lineRHLR_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
```
Plus whatever the lead decides for `fn_800C1EE0` (a friend declaration, as
tested here, or promoting it to a private static member -- open question).
