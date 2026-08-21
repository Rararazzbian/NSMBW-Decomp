// Deliverable for batch d4_misc (agent scope: GetPos, acm_angle, fn_800C15B0,
// circle_nextpos_set, fn_800C3BA0, fn_800C3BF0, calc_rotate_to_circle_rev,
// calc_rotate_to_circle_prev).
//
// ALL 7 authored functions below are BYTE-EXACT against retail (verified via
// wip/line_mng_shared/tally.py). fn_800C15B0 was found ALREADY PRESENT in
// wip/fix_bigtwo/d_line_mng.cpp (as `setArrElem_800C15B0`) and already
// byte-exact -- no work needed there, included below only as a placement
// anchor / for completeness, NOT as new work.
//
// Compiled and measured as part of the full TU (my own workspace copy,
// wip/gapA/d4_misc/d_line_mng.cpp) -- see report for the before/after tally
// and the regression check against the rest of the file.
//
// SHADOW HEADER CHANGE REQUIRED (see wip/gapA/d4_misc/shadow_include/game/bases/d_line_mng.hpp):
//   acm_angle()'s return type must change from `void` to `u16`. Evidence:
//   target.txt shows a `beqlr` (conditional RETURN) right after the value is
//   computed into r3 and masked to 16 bits on BOTH paths -- a void function
//   has no reason to keep a value alive in r3 across a return point, and an
//   unused arithmetic expression would be dead-code-eliminated entirely under
//   -O4, not computed+masked unconditionally. Compiling it as `u16` is what
//   produced the byte-exact match; `void` was never tried since the evidence
//   already rules it out structurally.

// ---------------------------------------------------------------------------
// PLACEMENT 1: insert between dLineMng_c::move() and dLineMng_c::SetPos() in
// wip/fix_bigtwo/d_line_mng.cpp (retail address order: move -> GetPos ->
// SetPos -> CalcAdjustPosY). Target: GetPos__10dLineMng_cCFv, 5 words.
// Draft: 5 words. BYTE-EXACT.
// ---------------------------------------------------------------------------
mVec2_c dLineMng_c::GetPos() const
{
    return mPos;
}

// ---------------------------------------------------------------------------
// PLACEMENT 2: insert between dLineMng_c::SetBaseSpeed() and the existing
// fn_800C15B0 comment block (retail address order: SetBaseSpeed -> acm_angle
// -> fn_800C15B0 -> start_line_move). Target: acm_angle__10dLineMng_cCFv,
// 9 words. Draft: 9 words. BYTE-EXACT.
//
// Requires the shadow-header return-type change noted above.
//
// Residual note for posterity: the FIRST attempt (no hoisted local, just
// `if (!mReverse) return mAngle + 0x4000; return mAngle - 0x4000;`) compiled
// to 11 words with a branch-around-and-two-reloads shape -- WRONG. Retail
// loads mAngle ONCE, unconditionally, before the branch (visible as an
// `lhz` at the top, ahead of the `cmpwi`), and reuses that one register in
// both the early-return (`beqlr`) and fallthrough paths. Hoisting the field
// read into a named local up front is what reproduces this exactly.
// ---------------------------------------------------------------------------
u16 dLineMng_c::acm_angle() const
{
    u16 angle = mAngle;
    if (!mReverse) {
        return angle + 0x4000;
    }
    return angle - 0x4000;
}

// ---------------------------------------------------------------------------
// fn_800C15B0 -- ALREADY PRESENT in wip/fix_bigtwo/d_line_mng.cpp as
// `setArrElem_800C15B0` (author_core), already byte-exact (7/7 words),
// confirmed via tally.py's content-based fallback pairing. NOT reproduced
// here; no action needed. Caveat in the brief is correct.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// PLACEMENT 3: insert between the end of fn_800C31C0 and the
// "author_core: fn_800C3B20 / fn_800C3B60" comment block (retail address
// order: fn_800C31C0 -> circle_nextpos_set -> fn_800C3B20).
// Target: circle_nextpos_set__10dLineMng_cFRC7mVec2_cf, 47 words.
// Draft: 47 words. BYTE-EXACT.
//
// Uses the project's standard nw4r::math::CosIdx/SinIdx(short) idiom
// (include/lib/nw4r/math/math_triangular.h + math_arithmetic.h): these
// inline down to the paired-single "store u16 to stack, psq_l reload as
// float, multiply by 1/256" sequence, which is exactly the `sth`+`psq_l
// ...,1,qr3`+`fmuls ...,(1/256)` shape target.txt shows immediately before
// each `bl CosFIdx__Q24nw4r4mathFf` / `bl SinFIdx__Q24nw4r4mathFf`. mAngle
// (a u16 field) is passed to CosIdx/SinIdx's `short` parameter with NO
// explicit cast in source -- that implicit u16->short conversion is what
// makes the compiler choose a sign-extending `lha` for this particular
// read of mAngle, vs. the plain `lhz` acm_angle() uses for the same field
// when it needs the raw unsigned value. Y is computed before X throughout
// (matches retail's field/store order exactly: mUnitBasePos.y+pos.y first,
// then mUnitBasePos.x+pos.x, stores 0x5c/0x58/0x40/0x44 in that order).
// ---------------------------------------------------------------------------
void dLineMng_c::circle_nextpos_set(const mVec2_c &pos, f32 radius)
{
    mUnk58.y = mUnitBasePos.y + pos.y;
    mUnk58.x = mUnitBasePos.x + pos.x;
    mPos.x = mUnk58.x;
    mPos.y = mUnk58.y;
    mPos.x = mPos.x + radius * nw4r::math::CosIdx(mAngle);
    mPos.y = mPos.y + radius * nw4r::math::SinIdx(mAngle);
}

// ---------------------------------------------------------------------------
// PLACEMENT 4: insert between the end of fn_800C3B60 and the
// "author_mov: mov_to_*/mov_frm_* quadruplets" comment block (retail
// address order: fn_800C3B60 -> fn_800C3BA0 -> fn_800C3BF0 ->
// calc_rotate_to_circle_rev -> calc_rotate_to_circle_prev -> mov_to_rightupper).
// All four below are BYTE-EXACT. Definition order matters here (per
// AGENT_CONTEXT.md) -- keep this exact sequence.
//
// fn_800C3BA0: target 18 words, draft 18 words.
// fn_800C3BF0: target 8 words (listed as 7 in the brief's caveat text --
//   recount from target.txt gives 8; trust the measured tally, not the
//   brief), draft 8 words.
// calc_rotate_to_circle_rev: target 27 words, draft 27 words.
// calc_rotate_to_circle_prev: target 27 words, draft 27 words.
//
// Both static helpers are plain file-scope functions taking a single u16
// argument (mAngle's value), no `this` -- called with a bare int argument
// from both callers below, no class access needed, so no friend declaration
// or header change required.
//
// fn_800C3BA0 reduces a BAM angle down by 0x4000 while it is >= 0x4000,
// compiled as a guarded, trip-count-computed, duff's-device-unrolled ctr
// loop (MWCC's standard transform when it can prove a small bound on a
// `while` loop's iteration count -- here, at most 3, since the argument's
// value never exceeds 0xFFFF). fn_800C3BF0 mirrors it for wrapping UP past
// 0xC000, but compiles as a plain test-at-bottom loop -- MWCC did not find
// a closed-form trip count for that direction, so don't try to force the
// unrolled shape onto it.
//
// Residual note for posterity: the FIRST attempt at the two
// calc_rotate_to_circle_* functions (calling `fn_800C3BF0(mAngle)` inline,
// once per branch, no cast) compiled to 24 words each vs retail's 27 -- both
// missing the SAME two things: (1) retail loads mAngle ONCE, unconditionally,
// before the `reverse` branch, and reuses that register (`mr r3,r0`) in
// each arm, rather than reloading the field fresh inside each arm; (2)
// retail masks the helper's return value back to unsigned 16 bits
// (`clrlwi r0,r3,16`) before combining it with `target` -- since the helpers
// return `short` (sign-extended via `extsh` internally), this only appears
// with an EXPLICIT `(u16)` cast at the call site; without it, C++'s usual
// arithmetic conversions promote the `short` to a plain (still signed) `int`
// and no mask is emitted. Both fixes together closed the residual exactly.
// ---------------------------------------------------------------------------
static short fn_800C3BA0(u16 angle)
{
    while (angle >= 0x4000) {
        angle -= 0x4000;
    }
    return (short)angle;
}

static short fn_800C3BF0(u16 angle)
{
    while (angle < 0xC000) {
        angle -= 0x4000;
    }
    return (short)angle;
}

void dLineMng_c::calc_rotate_to_circle_rev(u16 target, bool reverse)
{
    u16 angle = mAngle;
    if (reverse) {
        mAngle = target + (u16)fn_800C3BF0(angle);
    } else {
        mAngle = target - (u16)fn_800C3BA0(angle);
    }
}

void dLineMng_c::calc_rotate_to_circle_prev(u16 target, bool reverse)
{
    u16 angle = mAngle;
    if (reverse) {
        mAngle = target - (u16)fn_800C3BF0(angle);
    } else {
        mAngle = target + (u16)fn_800C3BA0(angle);
    }
}
