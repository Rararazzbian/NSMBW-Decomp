#include <game/bases/d_line_mng.hpp>
#include <cmath>
#include <nw4r/math/math_triangular.h>

// MERGE NOTE (author_core): the real <game/bases/d_bc.hpp> pulls in
// d_bg_ctr.hpp/d_actor.hpp, which fail to compile standalone outside the full
// project context (undefined dBg_ctr_c, illegal 'virtual' outside a class).
// This minimal forward declaration reproduces the exact mangled signatures
// (getUnitType__5dBc_cFffUc / getUnitKind__5dBc_cFffUc) from the real
// include/game/bases/d_bc.hpp:217-218 without dragging in the rest of the
// actor hierarchy. ASSUMED HELPER SIGNATURE -- ports cleanly once the real
// header can be included in-tree.
class dBc_c {
public:
    static u32 getUnitType(float x, float y, u8 layer);
    static u32 getUnitKind(float x, float y, u8 layer);
};

STATE_DEFINE(dLineMng_c, Idle);
STATE_DEFINE(dLineMng_c, FallDown);
STATE_DEFINE(dLineMng_c, Left45);
STATE_DEFINE(dLineMng_c, Right45);
STATE_DEFINE(dLineMng_c, Side);
STATE_DEFINE(dLineMng_c, Height);
STATE_DEFINE(dLineMng_c, CornerHeightLine);
STATE_DEFINE(dLineMng_c, CornerSideLine);
STATE_DEFINE(dLineMng_c, Left30Left);
STATE_DEFINE(dLineMng_c, Left30Right);
STATE_DEFINE(dLineMng_c, Right30Left);
STATE_DEFINE(dLineMng_c, Right30Right);
STATE_DEFINE(dLineMng_c, Left60Up);
STATE_DEFINE(dLineMng_c, Left60Down);
STATE_DEFINE(dLineMng_c, Right60Down);
STATE_DEFINE(dLineMng_c, Right60Up);
STATE_DEFINE(dLineMng_c, Circle);
STATE_DEFINE(dLineMng_c, Circle2x2Leftup);
STATE_DEFINE(dLineMng_c, Circle2x2Rightup);
STATE_DEFINE(dLineMng_c, Circle2x2LeftDown);
STATE_DEFINE(dLineMng_c, Circle2x2RightDown);
STATE_DEFINE(dLineMng_c, Circle4x4Rightup);
STATE_DEFINE(dLineMng_c, Circle4x4LeftUp);
STATE_DEFINE(dLineMng_c, Circle4x4LeftDown);
STATE_DEFINE(dLineMng_c, Circle4x4RightDown);

#pragma opt_strength_reduction off
const float dLineMng_c::smc_UNIT_SIZE_X = 16.0f;

dLineMng_c::dLineMng_c() :
    mStateMgr(*this, sStateID::null)
{}

// ===========================================================================
// author_core: lifecycle core
// ===========================================================================

void dLineMng_c::init(const mVec2_c &pos, f32 speed, int lineType, u8 param)
{
    mLineType = (u8)lineType;
    init_term_ck_pos();
    mSpeed.set(0.0f, 0.0f);
    mPos.x = pos.x;
    mPos.y = pos.y;
    mOldPos.x = pos.x;
    mOldPos.y = pos.y;
    mBaseSpeed = speed;
    mUnitBasePos.x = (f32)(int)(pos.x / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X;
    mUnitBasePos.y = (f32)(int)(pos.y / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X;
    mAngle = 0;
    mType = param;
    mReverse = 0;
    mStateMgr.changeState(StateID_Idle);
    mUnk67 = 0;
}

void dLineMng_c::move()
{
    if (!mUnk67) {
        mStateMgr.executeState();
        mOldPos.x = mPos.x;
        mOldPos.y = mPos.y;
    }
}

// PLACEMENT 1: insert between dLineMng_c::move() and dLineMng_c::SetPos() in
// wip/fix_bigtwo/d_line_mng.cpp (retail address order: move -> GetPos ->
// SetPos -> CalcAdjustPosY). Target: GetPos__10dLineMng_cCFv, 5 words.
// Draft: 5 words. BYTE-EXACT.
// ---------------------------------------------------------------------------
mVec2_c dLineMng_c::GetPos() const
{
    return mPos;
}

void dLineMng_c::SetPos(const mVec2_c &pos)
{
    mPos.x = pos.x;
    mPos.y = pos.y;
}

f32 dLineMng_c::CalcAdjustPosY(f32 a, f32 b)
{
    // DECLARE-EARLY / ASSIGN-LATE. The callee-saved FP registers f31..f28 are
    // handed out in DECLARATION order, but the instruction schedule follows
    // ASSIGNMENT order, and here retail needs the two to disagree: absB is
    // f30 and x is f29 (declaration order absB-then-x), yet `fabs f0,f28` /
    // `frsp f30,f0` are scheduled AFTER the first `bl GetPos` (assignment
    // order x-then-absB). Writing `f32 absB = fabs(b);` above `f32 x` gets the
    // registers right but hoists the fabs into the prologue, which then kills
    // the `fmr f28, f2` that preserves `b` across the call and comes out
    // 127w instead of 128w. Splitting the declaration from the assignment
    // satisfies both rules and makes the function BYTE-EXACT (128/128).
    f32 origSpeed = mBaseSpeed;
    f32 absB;
    f32 x = GetPos().x;
    absB = std::fabs(b);
    f32 y = GetPos().y;
    if (std::fabs(a - x) < 0.01f) {
        return y;
    }
    if (a > x) {
        SetBaseSpeed(absB);
        int count = 0x4000;
        while (x <= a) {
            if (mStateMgr.getStateID()->isEqual(StateID_FallDown)) break;
            move();
            if (--count == 0) break;
            x = GetPos().x;
            y = GetPos().y;
        }
    } else {
        SetBaseSpeed(-absB);
        int count = 0x4000;
        while (x <= a) {
            if (mStateMgr.getStateID()->isEqual(StateID_FallDown)) break;
            move();
            if (--count == 0) break;
            x = GetPos().x;
            y = GetPos().y;
        }
    }
    SetBaseSpeed(origSpeed);
    return y;
}

void dLineMng_c::SetBaseSpeed(f32 speed)
{
    if (mReverse) speed = -speed;
    mBaseSpeed = speed;
}

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

/// @unofficial fn_800C15B0 (0x1C bytes, 7 words). Shape: arr[idx] = *src for
/// an 8-byte-stride array (author_core). Sits between acm_angle() and
/// start_line_move() in the target's .text address order. fn_800C31C0 (now
/// authored, see below) does NOT call it -- fix_bighelper enumerated all 23
/// of fn_800C31C0's callees and none is this function; grepping the entire
/// 182-function target for address 0x800C15B0 also finds zero callers
/// anywhere in this TU.
///
/// PROVEN non-static (external/global linkage), MEASURED, not merely
/// theorised: target.txt's OWN listing tags it `.fn fn_800C15B0, global`
/// (line 549, between acm_angle's `global` entry and start_line_move's),
/// so the target binary itself carries this as global binding, not an
/// inference from ELF section layout. Branches in a DOL are PC-relative, so
/// an absolute-word search for 0x800C15B0 (as done for the caller question
/// above) cannot find a `bl` from another translation unit -- consistent
/// with "no caller in this 182-function unit" AND "global linkage" both
/// being true at once: it is called from a still-un-decompiled TU. Declaring
/// it WITHOUT `static` here (this line) is enough on its own: MWCC emits an
/// external-linkage function unconditionally, whether or not this TU calls
/// it, and the result is 7 words, content-identical to target (confirmed:
/// tally.py's content-based fallback pairs it, MERGE2.md section 4).
void setArrElem_800C15B0(mVec2_c *arr, const mVec2_c *src, int idx)
{
    arr[idx] = *src;
}

void dLineMng_c::start_line_move()
{
    double t = std::fmod(mPos.x, 16.0);
    int triggerFallDown = 0;
    mUnk6a = triggerFallDown;
    // `-0.1f`/`0.1f`, NOT `-0.1`/`0.1`. `t` is a double, so both literals widen
    // and the pool entries are `lfd`-loaded either way -- but the BYTES differ:
    // retail's are BFB99999A0000000 / 3FB99999A0000000, which is `(double)(0.1f)`
    // with the float's trailing zeros, not the exact double 0.1
    // (BFB999999999999A). An `lfd`'s offset field is zeroed in the disassembly,
    // so the wrong constant is byte-identical to the right one and the match gate
    // cannot see it; `tools/auto_decomp/poolcheck.py` is what caught this.
    if (t > -0.1f && t < 0.1f) {
        mVec2_c snap;
        // SAME REGISTER-CHOICE LEVER as the executeState_* mBaseSpeed fix, applied
        // to a fdivs instead of a fmuls: mPos.x/mPos.y only land in the SAME
        // registers as retail (f0/f5) if each has its own def-point immediately
        // ahead of the divide. Written as a bare `mPos.x / smc_UNIT_SIZE_X` operand
        // the member has no def and the two divisions come out with x and y
        // swapped between f0/f1 and f0/f5 vs retail. Gets this function bit-exact.
        f32 px = mPos.x;
        snap.x = (f32)(int)(px / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X - 16.0f;
        f32 py = mPos.y;
        snap.y = (f32)(int)(py / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X;
        u32 unitType = getLineUnitNo(snap.x, snap.y);
        if (unitType == 6) {
            mUnitBasePos.x = snap.x;
            mUnitBasePos.y = snap.y;
            mStateMgr.changeState(StateID_Height);
        } else if (unitType == 4) {
            mUnitBasePos.x = snap.x;
            mUnitBasePos.y = snap.y;
            mStateMgr.changeState(StateID_CornerHeightLine);
        } else {
            triggerFallDown = 1;
        }
    } else {
        triggerFallDown = 1;
    }
    if (triggerFallDown) {
        mStateMgr.changeState(StateID_FallDown);
    }
}

bool dLineMng_c::is_unit_circle2x2(ulong unitID)
{
    static const ulong d_unit[] = { 0x11, 0x12, 0x13, 0x14 };
    const ulong *p = d_unit;
    do {
        if (unitID == *p) return true;
        p++;
    } while (p < d_unit + 4);
    return false;
}

bool dLineMng_c::is_unit_circle4x4(ulong unitID)
{
    static const ulong d_unit[] = { 0x16, 0x17, 0x19, 0x1a, 0x1b, 0x1c, 0x1e, 0x1f };
    const ulong *p = d_unit;
    do {
        if (unitID == *p) return true;
        p++;
    } while (p < d_unit + 8);
    return false;
}

void dLineMng_c::change_dir()
{
    mBaseSpeed = -mBaseSpeed;
    mReverse ^= 1;
}

u32 dLineMng_c::getLineUnitNo(f32 x, f32 y)
{
    u8 result = 0;
    if (!dBc_c::getUnitType(x, y, 0)) {
        result = dBc_c::getUnitKind(x, y, 0);
    }
    return result;
}

// File-scope (not function-local): target's symbol names for these are bare
// "lbl_80359740"/"lbl_8042A270", not the "@LOCAL@init_term_ck_pos..." mangled
// form MWCC emits for a genuinely function-local static -- AGENT_CONTEXT's
// "function-local static -> file scope" lever, confirmed by the symbol shape
// itself rather than assumed. From wip/fix_bighelper.
static mVec2_POD_c s_dDir[4];
static s8 s_dDirInit = 0;  // s8, not u8 -- gives retail's extsb. test rather than lbz+cmpwi

void dLineMng_c::init_term_ck_pos()
{
    mVec2_c *p = mDirVec;
    mVec2_c *end0 = p + 3;
    do {
        p->x = 0.0f;
        p->y = 0.0f;
        p++;
    } while ((u32)p < (u32)end0);

    if (!s_dDirInit) {
        s_dDir[0].set(-0.1f, 0.1f);
        s_dDir[1].set(0.1f, 0.1f);
        s_dDir[2].set(-0.1f, -0.1f);
        s_dDir[3].set(0.1f, -0.1f);
        s_dDirInit = 1;
    }

    const mVec2_POD_c *q = s_dDir;
    mVec2_c *end1 = end0 + 4;
    do {
        p->x = q->x;
        p->y = q->y;
        q++;
        p++;
    } while ((u32)p < (u32)end1);
}

bool dLineMng_c::check_term()
{
    mVec2_c *p = mDirVec;
    mVec2_c *end = mDirVec + 7;
    do {
        // Named local, not an inline rvalue -- target has an unread stfs
        // pair right before the `bl getLineUnitNo` (writes to r1+0x8/+0xc,
        // never loaded back), which only happens when the value is first
        // assigned to a real local rather than passed as a bare expression.
        //
        // The four leaf loads are split across TWO independent orderings and
        // the shape below is the only one of ~25 tested that reproduces both
        // at once (BYTE-EXACT, 73/73):
        //   * mPos.y/mPos.x each get a DEF-POINT of their own (`by`/`bx`)
        //     ahead of the adds (lever 12). Without them the fadds folds into
        //     a compound accumulate and the result lands in the left operand's
        //     register instead of the right's. Their register numbers ASCEND
        //     in DEFINITION order -- by=f2, bx=f3 -- so `by` must be declared
        //     FIRST even though the X component is used first.
        //   * `p->x`/`p->y` stay BARE leaves (no def-point) and are numbered
        //     DESCENDING in EVALUATION order, so the X add must be written
        //     FIRST to give p->x=f1, p->y=f0.
        // Hence: declare Y's base first, then compute X's sum first.
        // The two quantise stores must then be X then Y, which is what puts
        // X in the 0x10/0x18 conversion slot pair and stores it to r1+0x8.
        f32 by = mPos.y;
        f32 bx = mPos.x;
        f32 sx = bx + p->x;
        f32 sy = by + p->y;
        mVec2_c testPos;
        testPos.x = (f32)(int)(sx / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X;
        testPos.y = (f32)(int)(sy / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X;
        if (getLineUnitNo(testPos.x, testPos.y) == 0x22) {
            if (mLineType == 0) {
                change_dir();
            }
            mUnk6a = 1;
            return true;
        }
        p++;
    } while (p < end);
    return false;
}

// ===========================================================================
// author_geom: collision/intersection geometry family
// ===========================================================================

// The literal 0.0f in the two fcmpu compares below (here and in line_cross_chk3)
// canonicalises to the FIRST fcmpu operand slot when spelled as a bare token --
// confirmed by testing every spelling (0.0f==x, x!=0.0f negated via if/else,
// comparing against a live variable instead) with NO effect on operand order.
// Passing it through this identity inline defeats that canonicalisation: at
// parse time the argument is a plain parameter (not a syntactic float-literal
// AST node), and only becomes the constant 0.0f after inlining/const-prop --
// the same "route 3" mechanism AGENT_CONTEXT.md lever 11 documents for fmuls,
// applied here to fcmpu. MEASURED to flip both residuals to variable-first,
// matching target's fcmpu operand order exactly, with no other instruction
// changed.
static inline f32 zero_ref(f32 z) { return z; }

bool dLineMng_c::line_cross_slope_check(const mVec2_c &a, const mVec2_c &b, f32 &slope, f32 &intercept) {
    // NAMED AGGREGATE local, not two scalar f32 temps -- target allocates a
    // real 0x10 stack frame here and has an unread stfs PAIR for dx/dy right
    // before the dx==0.0f branch (both fields stored, neither ever reloaded,
    // since the register copies stay live for the rest of the function).
    // That is the same "store a real local, then keep using the live
    // register copy" idiom as check_term()'s testPos -- a bare pair of f32
    // locals never gets memory-backed at all (0 frame, exactly what a plain
    // `f32 dx/dy` compiled to here), so the vector needs to be one genuine
    // mVec2_c object for the stack slot to exist.
    mVec2_c d(b.x - a.x, b.y - a.y);
    // Branch polarity per target's beq/b-around shape (lever 5): the
    // success arm is the fallthrough `if`, not an early `return false`.
    if (d.x != zero_ref(0.0f)) {
        slope = d.y / d.x;
        intercept = b.y - slope * b.x;
        return true;
    }
    return false;
}

bool dLineMng_c::line_cross_range_check(f32 a, f32 b, f32 v) {
    f32 lo, hi;
    if (b < a) {
        lo = b;
        hi = a;
    } else {
        lo = a;
        hi = b;
    }
    lo -= 0.1f;
    hi += 0.1f;
    if (v >= lo) {
        if (v <= hi) {
            return true;
        }
    }
    return false;
}

bool dLineMng_c::line_cross_chk1(f32 p1, f32 p2, const mVec2_c &p3, mVec2_c p4, mVec2_c p5, mVec2_c &out) {
    p4.x -= p3.x;
    p4.y -= p3.y;
    p5.x -= p3.x;
    p5.y -= p3.y;

    f32 slope, intercept;
    // Residual was 2 instructions: both fcmpu's had their two operands
    // (the computed value and the 0.0f literal) in the SAME two registers as
    // target, just encoded in the opposite order (literal-first here vs
    // value-first in target), even though the source already spelled
    // `d != 0.0f` / `intercept != 0.0f` with the value on the left --
    // confirming this is not a source-text-order effect (that axis was
    // already tested and correctly found immune). The declaration-order axis
    // WAS the lever: a bare `f32 zero;` declared here, assigned 0.0f only at
    // first use just inside the branch, makes both compares byte-exact.
    // A syntactic `0.0f` literal is folded through MWCC's literal-hoisting
    // canonicaliser BEFORE the fcmpu operand-order decision is made (same
    // canonicaliser lever 11 documents for fmuls); a value that is only a
    // literal after const-prop (this local) bypasses it, exactly like lever
    // 11's route 3. Gets the function BYTE-EXACT (121/121).
    f32 zero;
    if (line_cross_slope_check(p4, p5, slope, intercept)) {
        f32 d = p1 - slope;
        zero = 0.0f;
        if (d != zero) {
            out.x = intercept / d;
            out.y = p1 * out.x;
        } else {
            if (intercept != zero) {
                return false;
            }
            out.x = p5.x;
            out.y = p5.y;
        }

        if (out.x >= -0.1f && out.x <= p2 + 0.1f) {
            if (line_cross_range_check(p4.x, p5.x, out.x)) {
                out.x += p3.x;
                out.y += p3.y;
                return true;
            }
        }
    } else {
        if (p5.x >= 0.0f && p5.x < p2) {
            f32 x5 = p5.x;
            out.x = x5;
            out.y = p1 * x5;
            if (line_cross_range_check(p4.y, p5.y, out.y)) {
                out.x += p3.x;
                out.y += p3.y;
                return true;
            }
        }
    }
    return false;
}

bool dLineMng_c::line_cross_chk2(f32 p1, const mVec2_c &p3, mVec2_c p4, mVec2_c p5, f32 &out) {
    p4.x -= p3.x;
    p4.y -= p3.y;
    p5.x -= p3.x;
    p5.y -= p3.y;

    if (p4.x != 0.0f && p5.x != 0.0f) {
        if ((p4.x < 0.0f && p5.x < 0.0f) || (p4.x >= 0.0f && p5.x >= 0.0f)) {
            return false;
        }
    }

    f32 slope, intercept;
    if (line_cross_slope_check(p4, p5, slope, intercept)) {
        f32 v = intercept;
        if (v >= -0.1f && v <= 0.1f + p1) {
            if (line_cross_range_check(p4.y, p5.y, v)) {
                out = v;
                return true;
            }
        }
    } else {
        if (p5.x == 0.0f) {
            if (p5.y >= 0.0f && p5.y < p1) {
                out = p5.y;
                return true;
            }
        }
    }
    return false;
}

bool dLineMng_c::line_cross_chk3(f32 p1, const mVec2_c &p2, const mVec2_c &p3) {
    // d3 = p3.x*p3.x, THEN accumulated via += / -=, not one flat expression --
    // this keeps d3 pinned in its own dedicated register (f4 in target) across
    // both the y^2 add and the p1 subtract, matching target's stable
    // accumulator; a flat `a*a + b*b - c` expression lets -O4 fold it into
    // whichever register the y^2 term happened to land in instead.
    f32 d3 = p3.x * p3.x;
    d3 += p3.y * p3.y;
    d3 -= p1;
    // zero_ref() bypass -- see line_cross_slope_check's comment above it.
    if (d3 == zero_ref(0.0f)) {
        return true;
    }
    f32 d2 = p2.x * p2.x;
    d2 += p2.y * p2.y;
    d2 -= p1;
    // First block: plain `if (d3 < 0.0f) { ... }`, no else -- the fallthrough
    // (d3>=0) needs no explicit branch of its own, it just falls into the
    // second block below. Lever: direct `<`/`>` (or its negation) lowers to
    // a bare hardware flag branch (bge here); it's only a literal `>=`/`<=`
    // that MWCC lowers via cror.
    if (d3 < 0.0f) {
        if (d2 >= 0.0f) {
            goto ok;
        }
    }
    // Second block re-tests d3's sign -- this is a genuinely redundant
    // retest reached both by falling through the first block's fallthrough
    // (d3>=0) AND by falling OUT of it (d3<0 but d2<0 too). Both `goto ok;`
    // sites share the SAME physical return-true block in target (one is a
    // forward branch into it, the other a plain fallthrough) -- a bare
    // `return true;` written twice does NOT get tail-merged by MWCC the
    // same way; it has to be one label both paths jump/fall into.
    if (d3 >= 0.0f) {
        // Negated-LT (`!(d2<0.0f)`) so this specific occurrence lowers to a
        // plain bge (matches target's un-cror'd final compare), with the
        // return-true block sitting BEFORE return-false in address order --
        // reorder the two labels to put `ok:` first, `fail:` last.
        if (!(d2 < 0.0f)) {
            goto fail;
        }
    } else {
        // Only reached via d3<0 && d2<0 (the first block's fallthrough) --
        // that combination is a genuine failure, not an implicit "ok".
        goto fail;
    }
ok:
    return true;
fail:
    return false;
}

// @unofficial unnamed file-scope helper (target `fn_800C1EE0`, author_geom).
// Reconstructed from width_cross_chk's caller side and its own body: takes a
// dLineMng_c* that passes straight through from the caller's own `this`
// (unmodified before the call), so it is a free function, not a class member
// (member statics in this unit -- line_cross_chk1 etc. -- carry real mangled
// names in the target; this one has none at all). Needs `friend` access to
// touch mPos/mUnitBasePos -- see the MERGE-LOCAL header addition.
bool fn_800C1EE0(dLineMng_c *pThis, f32 a, f32 b, const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3, const mVec2_c &origin) {
    mVec2_c out;
    // Pass p2/p3 straight through as the by-value args -- NOT via a named
    // p2c/p3c local first. line_cross_chk1's p4/p5 are by-value copies; a
    // named intermediate local gets its own stack slot AND the compiler
    // still separately materialises the outgoing-argument copy for the
    // call, doubling the four stfs stores and costing 0x10 bytes of frame.
    // Passing p2/p3 directly lets the by-value parameter construct straight
    // into the argument save area, one store each, matching the target's
    // single stfs sequence and its 0x30 frame (this fn_800C1EE0 was 0x40).
    bool result = dLineMng_c::line_cross_chk1(a, b, origin, p2, p3, out);
    if (result) {
        pThis->mPos.x = out.x;
        pThis->mPos.y = out.y;
        pThis->mUnitBasePos.x = p1.x;
        pThis->mUnitBasePos.y = p1.y;
    }
    return result;
}

bool dLineMng_c::height_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // AGGREGATE COPY + compound assignment (levers 10/11), same as
    // lineB_cross_chk's origin -- both fields need an arithmetic op here, so
    // the plain 2-arg constructor leaves the fadds/fsubs operands
    // literal-first and swaps the p1.y/p1.x load order vs target.
    mVec2_c pt = p1;
    pt.x += 16.0f;
    pt.y -= 16.0f;
    f32 outY;
    bool result = line_cross_chk2(16.0f, pt, p2, p3, outY);
    if (result) {
        mPos.x = pt.x;
        // Write the sum BACK into outY itself, not a fresh local -- outY is
        // address-taken (passed by reference into line_cross_chk2), so it
        // already has real stack storage, and target has an unread stfs to
        // exactly that slot right after the add (write-only, never
        // reloaded back, since mPos.y's store below uses the live register
        // copy directly). A fresh separate local optimises away entirely
        // with nothing to spill.
        outY += pt.y;
        mPos.y = outY;
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
    // AGGREGATE COPY, not two scalar field writes -- lever 10. The member-wise
    // form front-loads the `a` literal and reverses the store order.
    mVec2_c origin = p1;
    origin.y -= 16.0f;
    return fn_800C1EE0(this, 0.0f, 16.0f, p1, p2, p3, origin);
}

// author_geom: line0..line4 (batch d1_lines_a). All-const-ref signature per
// shadow header (game/bases/d_line_mng.hpp:91-95) -- distinct from the
// lineF/circle/lineRH family below, which takes p2/p3 BY VALUE because they
// mutate them (subtract an origin) before use. line0-line4 never mutate p2/p3
// themselves -- they only ever forward them untouched into fn_800C1EE0/
// width_cross_chk/height_cross_chk, so a const ref suffices and the mangled
// name in target.txt confirms it (RC7mVec2_cRC7mVec2_cRC7mVec2_c, all three
// reference-qualified).
bool dLineMng_c::line0_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin = p1;
    origin.y -= 16.0f;
    bool result = fn_800C1EE0(this, 1.0f, 16.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left45);
    }
    return result;
}

bool dLineMng_c::line1_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = fn_800C1EE0(this, -1.0f, 16.0f, p1, p2, p3, p1);
    if (result) {
        mStateMgr.changeState(StateID_Right45);
    }
    return result;
}

bool dLineMng_c::line3h_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = width_cross_chk(p1, p2, p3);
    if (result) {
        mStateMgr.changeState(StateID_CornerSideLine);
    }
    return result;
}

bool dLineMng_c::line3v_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = height_cross_chk(p1, p2, p3);
    if (result) {
        mStateMgr.changeState(StateID_CornerHeightLine);
    }
    return result;
}

bool dLineMng_c::line4_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = width_cross_chk(p1, p2, p3);
    if (result) {
        mStateMgr.changeState(StateID_Side);
    }
    return result;
}

bool dLineMng_c::line5_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = height_cross_chk(p1, p2, p3);
    if (result) {
        mStateMgr.changeState(StateID_Height);
    }
    return result;
}

bool dLineMng_c::line7_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin = p1;
    origin.y -= 16.0f;
    bool result = fn_800C1EE0(this, 0.5f, 16.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left30Left);
    }
    return result;
}

bool dLineMng_c::line8_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin = p1;
    origin.y -= 8.0f;
    bool result = fn_800C1EE0(this, 0.5f, 16.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left30Right);
    }
    return result;
}

bool dLineMng_c::line9_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = fn_800C1EE0(this, -0.5f, 16.0f, p1, p2, p3, p1);
    if (result) {
        mStateMgr.changeState(StateID_Right30Left);
    }
    return result;
}

bool dLineMng_c::lineA_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin = p1;
    origin.y -= 8.0f;
    bool result = fn_800C1EE0(this, -0.5f, 16.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Right30Right);
    }
    return result;
}

bool dLineMng_c::lineB_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // AGGREGATE COPY (AGENT_CONTEXT lever 10) + compound assignment (lever 11):
    // this is what puts BOTH the member-first fadds/fsubs operand order AND
    // the clustered-then-computed load schedule the target shows. A plain
    // `mVec2_c origin(p1.x + 8.0f, p1.y - 16.0f);` matches the load schedule
    // but leaves the adds/subs literal-first (wrong); a field-by-field
    // `origin.x = p1.x + 8.0f; origin.y = p1.y - 16.0f;` gets neither.
    mVec2_c origin = p1;
    origin.x += 8.0f;
    origin.y -= 16.0f;
    bool result = fn_800C1EE0(this, 2.0f, 8.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left60Up);
    }
    return result;
}

bool dLineMng_c::lineC_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // Here only Y needs an arithmetic op and X is a plain copy, so the
    // ordinary two-argument constructor already produces the exact target
    // shape (no lever-11 operand-order issue arises for a bare copy).
    mVec2_c origin(p1.x, p1.y - 16.0f);
    bool result = fn_800C1EE0(this, 2.0f, 8.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left60Down);
    }
    return result;
}

bool dLineMng_c::lineD_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // Mirror of lineB but only X is computed (+8.0f) and Y is a plain copy;
    // needs the same aggregate-copy + compound-assignment shape as lineB for
    // the member-first fadds.
    mVec2_c origin = p1;
    origin.x += 8.0f;
    bool result = fn_800C1EE0(this, -2.0f, 8.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Right60Down);
    }
    return result;
}

bool dLineMng_c::lineE_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // No origin computation at all -- p1 is passed straight through as both
    // the p1 and origin arguments (confirmed by the target's r7 <- r4 `mr`
    // and its smaller 0x10 stack frame; the three-argument versions above
    // need 0x20 for a real local).
    bool result = fn_800C1EE0(this, -2.0f, 8.0f, p1, p2, p3, p1);
    if (result) {
        mStateMgr.changeState(StateID_Right60Up);
    }
    return result;
}
bool dLineMng_c::lineF_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    f32 oy;
    f32 ox;
    f32 px;
    oy = p1.y - 8.0f;
    px = p1.x;
    ox = px;
    ox += 8.0f;
    origin.y = oy;
    origin.x = ox;
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
    f32 oy;
    f32 ox;
    f32 px;
    oy = p1.y - 16.0f;
    px = p1.x;
    ox = px;
    ox += 16.0f;
    origin.y = oy;
    origin.x = ox;
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
            return result;
        } else {
            return false;
        }
    }
    return result;
}

bool dLineMng_c::circle_ur2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.x = p1.x;
    origin.y = p1.y - 16.0f;
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
            return result;
        } else {
            return false;
        }
    }
    return result;
}

bool dLineMng_c::circle_dl2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y;
    origin.x = p1.x;
    origin.x += 16.0f;
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
            return result;
        } else {
            return false;
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
            angle += 0xbf00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle2x2RightDown);
            return result;
        } else {
            return false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHUR_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.x = p1.x;
    origin.y = p1.y - 32.0f;
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
            return result;
        }
        return false;
    }
    return result;
}

bool dLineMng_c::lineRHUL_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    f32 oy;
    f32 ox;
    f32 px;
    oy = p1.y - 32.0f;
    px = p1.x;
    ox = px;
    ox += 32.0f;
    origin.y = oy;
    origin.x = ox;
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
            return result;
        }
        return false;
    }
    return result;
}

bool dLineMng_c::lineRHLL_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y;
    origin.x = p1.x;
    origin.x += 32.0f;
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
            return result;
        }
        return false;
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
            angle += 0xbf00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle4x4RightDown);
            return result;
        }
        return false;
    }
    return result;
}

// ===========================================================================
// fn_800C31C0 -- unnamed file-scope helper, THE LARGEST FUNCTION IN THE UNIT
// (0x894 bytes, 549 words). Not a member (no offset-through-`this` access
// anywhere in its own body -- every field touch is `self->mPos`/`self->
// mOldPos` READS only). From wip/fix_bighelper.
//
// Shape: scans the 3x3 grid of UNIT_SIZE cells centred on (mPos snapped down
// by one cell), and for each of the 9 cells whose getLineUnitNo() id is in
// [0,0x20], dispatches through a dense switch (MWCC jump table, confirmed by
// reading raw bytes at .data:0x80316CA0 in original/wiimj2d.dol -- 33 function
// pointers, 3 of which point straight at the "miss" label) to the matching
// line/circle cross-check. The first one that returns true stops the whole
// scan (`goto found`, both found/not-found paths reach the same epilogue --
// the cross_chk callees themselves perform any state change as a side
// effect, fn_800C31C0 does none of its own).
//
// The exact case VALUE order below (1,2,4,5,6,9,8,11,10,12,13,14,15,16,18,17,
// 20,19,26,24,23,22,21,25,27,29,30,31,32,28) is not numeric -- it is the
// target's .text ADDRESS order for each case body, extracted directly from
// the jump table contents. Per AGENT_CONTEXT.md ("case LABEL declaration
// order sets body-block layout, independently of the dispatch comparison
// order"), declaring them in numeric order would still compile and still
// dispatch correctly, but would lay the case bodies out in the WRONG .text
// order and diff every byte from the first case onward.
static void fn_800C31C0(dLineMng_c *self)
{
    // The two-argument mVec2_c constructor, NOT two memberwise assignments.
    // MEASURED: memberwise (`posOld.x = ...; posOld.y = ...;`) runs each
    // float->int->float conversion to completion before starting the next;
    // the target INTERLEAVES the pair -- both fctiwz, both stfd, both xoris --
    // which is what an argument list produces, because MWCC evaluates
    // arguments RIGHT-TO-LEFT and both are live at once. Closing this also
    // moved `self` from r31 to r30 and the outer loop counter from r29 to
    // r31 on its own; that GPR rotation was never an independent defect.
    //
    // The Y-then-X local declarations are load-bearing twice over, and the
    // two effects are separable (measured: 62 -> 45 -> 25 differing):
    //   * ORDER: they fix the evaluation order to y-first, matching the
    //     target's `lfs 0x4c` before `lfs 0x48`.
    //   * DIRECTION: a bare member read is a leaf and MWCC numbers leaves
    //     DESCENDING from N-1 in evaluation order (y=f1, x=f0). The target
    //     numbers them ASCENDING (y=f0, x=f1), which is the signature of a
    //     leaf that has a DEF-POINT -- see AGENT_CONTEXT.md lever 12. An
    //     f32 local supplies that def-point. `int` locals give the ORDER but
    //     not the DIRECTION (the int result is the def, the float leaf is
    //     still bare) and stall at 45.
    f32 oy = self->mOldPos.y;
    f32 ox = self->mOldPos.x;
    mVec2_c posOld((f32)(int)ox, (f32)(int)oy);
    f32 ny = self->mPos.y;
    f32 nx = self->mPos.x;
    mVec2_c posNew((f32)(int)nx, (f32)(int)ny);

    // Grid-snap mPos down to its UNIT_SIZE cell, then step back one more cell
    // so the 3x3 scan below is centred on the unit mPos sits in.
    //
    // Three things here are load-bearing and they were the LAST four differing
    // words in the function. This is the exact shape of the byte-exact
    // `start_line_move` above (see its comment at the `f32 px = mPos.x;`
    // line) plus one addition, the reference:
    //
    //  1. `mp` -- a const REFERENCE to mPos, not a copy. Retail reloads mPos.x
    //     and mPos.y here even though posNew read them 18 words earlier (retail
    //     loads y-then-x for posNew and x-then-y here, so they are genuinely
    //     two separate reads). Reading `self->mPos.x` directly lets -O4 reuse
    //     the live `nx`/`ny` and the function comes out 547 words, 2 SHORT.
    //     An aggregate copy (`mVec2_c base = self->mPos;`) also defeats that
    //     CSE and gives the right LENGTH -- it is what this line used to be --
    //     but it folds each load into its own quotient register, and retail
    //     keeps them separate. The reference defeats the CSE without folding.
    //  2. `bx`/`by` -- the f32 locals give each dividend a DEF-POINT. Without
    //     them the dividend is a bare leaf, numbered descending and fused with
    //     the quotient: `lfs f1,0x40; fdivs f1,f1,f2`. With them retail's
    //     ascending four-value allocation appears: `lfs f0,0x40; fdivs f1,f0,f2`
    //     then `lfs f4,0x44` (f4, not f1, because f2 and f3 are still live)
    //     and `fdivs f0,f4,f2`. AGENT_CONTEXT.md lever 12, on a fdivs.
    //     NOTE the locals only work on top of (1): with the aggregate copy in
    //     place they are copy-propagated away and change nothing at all.
    //  3. The declarations are INTERLEAVED with their statements, matching the
    //     sibling. (Measured: both-up-front also reaches zero here, but the
    //     interleaved form is the one the original demonstrably uses.)
    mVec2_c base;
    const mVec2_c &mp = self->mPos;
    f32 bx = mp.x;
    base.x = (f32)(int)(bx / dLineMng_c::smc_UNIT_SIZE_X) * dLineMng_c::smc_UNIT_SIZE_X - 16.0f;
    f32 by = mp.y;
    base.y = (f32)(int)(by / dLineMng_c::smc_UNIT_SIZE_X) * dLineMng_c::smc_UNIT_SIZE_X - 16.0f;

    // Declaration ORDER sets the stack slots: corner lands at r1+0x120 and
    // pos at r1+0x118, which is what the target has. Declared the other way
    // round they swap and every `addi r4,r1,0x118` in the switch diffs (64
    // words, the single largest cause in the original 221-word residual).
    mVec2_c corner;
    mVec2_c pos;
    pos.x = base.x;
    pos.y = base.y;
    for (int j = 0; j < 3; j++) {
        pos.x = base.x;
        for (int i = 0; i < 3; i++) {
            u32 id = dLineMng_c::getLineUnitNo(pos.x, pos.y);
            // The `else` on every `break` below is NOT redundant. Written as
            // `if (X) goto found; break;` MWCC inverts the test and emits
            // `bne found; b <inc>`; the target has the uninverted
            // `beq <inc>; b found`. The explicit else suppresses the
            // inversion and closed all 28 sites at once (157 -> 101). The
            // proof it is the right shape was already in the file: case 4's
            // `||` pair matched from the start, and the LAST test of an `||`
            // chain emits exactly `beq <inc>; b found`.
            // Do NOT reach for `if (!X) break;` instead -- these return
            // `bool`, so the negation costs a `cntlzw`/`srwi.` pair and the
            // function grows to 566 words. Measured.
            switch (id) {
                case 1:
                    if (self->line0_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 2:
                    if (self->line1_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 4:
                    if (self->line3h_cross_chk(pos, posOld, posNew) ||
                        self->line3v_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 5:
                    if (self->line4_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 6:
                    if (self->line5_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 9:
                    if (self->line7_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 8:
                    if (self->line8_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 11:
                    if (self->line9_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 10:
                    if (self->lineA_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 12:
                    if (self->lineB_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 13:
                    if (self->lineC_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 14:
                    if (self->lineD_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 15:
                    if (self->lineE_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 16:
                    if (self->lineF_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 18:
                    if (self->circle_ul2_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 17:
                    if (self->circle_ur2_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 20:
                    if (self->circle_dl2_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 19:
                    if (self->circle_dr2_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 26:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.x -= 16.0f;
                    corner.y += 16.0f;
                    if (self->lineRHUR_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
                case 24:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.x -= 16.0f;
                    if (self->lineRHUR_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
                case 23:
                    if (self->lineRHUR_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 22:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.x -= 16.0f;
                    if (self->lineRHUL_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
                case 21:
                    if (self->lineRHUL_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 25:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.y += 16.0f;
                    if (self->lineRHUL_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
                case 27:
                    if (self->lineRHLL_cross_chk(pos, posOld, posNew)) goto found;
                    else break;
                case 29:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.y += 16.0f;
                    if (self->lineRHLL_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
                case 30:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.x -= 16.0f;
                    corner.y += 16.0f;
                    if (self->lineRHLL_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
                case 31:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.y += 16.0f;
                    if (self->lineRHLR_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
                case 32:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.x -= 16.0f;
                    corner.y += 16.0f;
                    if (self->lineRHLR_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
                case 28:
                    corner.x = pos.x;
                    corner.y = pos.y;
                    corner.x -= 16.0f;
                    if (self->lineRHLR_cross_chk(corner, posOld, posNew)) goto found;
                    else break;
            }
            pos.x += 16.0f;
        }
        pos.y += 16.0f;
    }
found:
    return;
}

// ===========================================================================
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

// author_core: fn_800C3B20 / fn_800C3B60 (unnamed file-scope helpers)
// Renamed here from author_core's clampPosX_800C3B20/clampPosY_800C3B60 to
// match author_states' assumed call-site names (fn_800C3B20/fn_800C3B60) --
// same bodies, same evidence. Declared `static` (internal linkage) to match
// author_states' assumption, since dtk shows no symbol name for them at all
// in the target -- consistent with the original binary's own stripped
// internal statics.
// ===========================================================================

/// @unofficial fn_800C3B20 (0x3C bytes, 15 words). Clamps mPos.x into
/// [mUnitBasePos.x, mUnitBasePos.x+16.0) against the near edge (0.1f margin
/// on the upper side).
static void fn_800C3B20(dLineMng_c *self)
{
    f32 baseX = self->mUnitBasePos.x;
    f32 posX = self->mPos.x;
    if (posX < baseX) {
        self->mPos.x = baseX;
    }
    f32 upper = baseX + 16.0f;
    f32 posX2 = self->mPos.x;
    if (!(posX2 >= upper)) {
        return;
    }
    self->mPos.x = upper - 0.1f;
}

/// @unofficial fn_800C3B60 (0x3C bytes, 15 words). Mirrors fn_800C3B20 for
/// the Y axis.
static void fn_800C3B60(dLineMng_c *self)
{
    f32 baseY = self->mUnitBasePos.y;
    if (self->mPos.y >= baseY) {
        self->mPos.y = baseY - 0.1f;
    }
    if (self->mPos.y < baseY - 16.0f) {
        self->mPos.y = baseY - 16.0f;
    }
}

// ===========================================================================
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

// author_mov: mov_to_*/mov_frm_* quadruplets
// ===========================================================================

bool dLineMng_c::mov_to_rightupper(ulong id, const mVec2_c &pos, bool reverse) {
    u32 unitID = getLineUnitNo(pos.x, pos.y);
    switch (unitID) {
    case 1:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 1) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Left45);
        }
        return true;
    case 4:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_CornerSideLine);
        return true;
    case 5:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 5) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Side);
        }
        return true;
    case 9:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Left30Left);
        return true;
    case 13:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Left60Down);
        return true;
    case 18:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0x7fff, reverse);
        } else {
            mAngle = 0x7fff;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2Leftup);
        return true;
    case 19:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0xc000, reverse);
        } else {
            mAngle = 0xc000;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2RightDown);
        return true;
    case 25:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.y += 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0x7fff, reverse);
        } else {
            mAngle = 0x7fff;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4LeftUp);
        return true;
    case 31:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.y += 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0xc000, reverse);
        } else {
            mAngle = 0xc000;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4RightDown);
        return true;
    }
    return false;
}

bool dLineMng_c::mov_to_rightlower(ulong id, const mVec2_c &pos, bool reverse) {
    u32 unitID = getLineUnitNo(pos.x, pos.y);
    if (unitID == 0x2) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 0x2) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Right45);
        }
        return true;
    }
    if (unitID == 0xb) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Right30Left);
        return true;
    }
    if (unitID == 0xf) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Right60Up);
        return true;
    }
    if (unitID == 0x11) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0x3fff, reverse);
        } else {
            mAngle = 0x3fff;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2Rightup);
        return true;
    }
    if (unitID == 0x14) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x8000, reverse);
        } else {
            mAngle = 0x8000;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2LeftDown);
        return true;
    }
    if (unitID == 0x17) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0x3fff, reverse);
        } else {
            mAngle = 0x3fff;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4Rightup);
        return true;
    }
    if (unitID == 0x1b) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x8000, reverse);
        } else {
            mAngle = 0x8000;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4LeftDown);
        return true;
    }
    return false;
}

bool dLineMng_c::mov_to_leftupper(ulong id, const mVec2_c &pos, bool reverse) {
    u32 unitID = getLineUnitNo(pos.x, pos.y);
    switch (unitID) {
    case 2:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 2) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Right45);
        }
        return true;
    case 5:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 5) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Side);
        }
        return true;
    case 6:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 6) {
            if (!reverse) change_dir();
            mStateMgr.changeState(StateID_Height);
        }
        return true;
    case 10:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Right30Right);
        return true;
    case 14:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Right60Down);
        return true;
    case 17:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x0, !reverse);
        } else {
            mAngle = 0x0;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2Rightup);
        return true;
    case 20:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0xbfff, !reverse);
        } else {
            mAngle = 0xbfff;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2LeftDown);
        return true;
    case 26:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.x -= 16.0f;
        mUnitBasePos.y += 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x0, !reverse);
        } else {
            mAngle = 0x0;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4Rightup);
        return true;
    case 30:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.x -= 16.0f;
        mUnitBasePos.y += 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0xbfff, !reverse);
        } else {
            mAngle = 0xbfff;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4LeftDown);
        return true;
    }
    return false;
}

bool dLineMng_c::mov_to_leftlower(ulong id, const mVec2_c &pos, bool reverse) {
    u32 unitID = getLineUnitNo(pos.x, pos.y);
    switch (unitID) {
    case 1:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 1) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Left45);
        }
        return true;
    case 4:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_CornerHeightLine);
        return true;
    case 6:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 6) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Height);
        }
        return true;
    case 8:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Left30Right);
        return true;
    case 12:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Left60Up);
        return true;
    case 18:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x4000, !reverse);
        } else {
            mAngle = 0x4000;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2Leftup);
        return true;
    case 19:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0xffff, !reverse);
        } else {
            mAngle = 0xffff;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2RightDown);
        return true;
    case 22:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.x -= 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x4000, !reverse);
        } else {
            mAngle = 0x4000;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4LeftUp);
        return true;
    case 28:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.x -= 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0xffff, !reverse);
        } else {
            mAngle = 0xffff;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4RightDown);
        return true;
    }
    return false;
}

void dLineMng_c::mov_frm_rightupper(const mVec2_c &pos, bool reverse) {
    u32 id = getLineUnitNo(pos.x, pos.y);
    mVec2_c tmp;
    tmp.x = pos.x;
    tmp.y = pos.y;
    tmp.x += 16.0f;
    if (mov_to_rightlower(id, tmp, reverse)) return;
    tmp.y = pos.y;
    tmp.y += 16.0f;
    if (mov_to_rightupper(id, tmp, reverse)) return;
    tmp.x = pos.x;
    if (mov_to_leftupper(id, tmp, !reverse)) return;
    mStateMgr.changeState(StateID_FallDown);
}

void dLineMng_c::mov_frm_leftlower(const mVec2_c &pos, bool reverse) {
    u32 id = getLineUnitNo(pos.x, pos.y);
    mVec2_c tmp;
    tmp.x = pos.x;
    tmp.y = pos.y;
    tmp.x -= 16.0f;
    if (mov_to_leftupper(id, tmp, reverse)) return;
    tmp.y = pos.y;
    tmp.y -= 16.0f;
    if (mov_to_leftlower(id, tmp, reverse)) return;
    tmp.x = pos.x;
    if (mov_to_rightlower(id, tmp, !reverse)) return;
    mStateMgr.changeState(StateID_FallDown);
}

void dLineMng_c::mov_frm_rightlower(const mVec2_c &pos, bool reverse) {
    u32 id = getLineUnitNo(pos.x, pos.y);
    mVec2_c tmp;
    tmp.x = pos.x;
    tmp.y = pos.y;
    tmp.x += 16.0f;
    if (mov_to_rightupper(id, tmp, reverse)) return;
    tmp.y = pos.y;
    tmp.y -= 16.0f;
    if (mov_to_rightlower(id, tmp, reverse)) return;
    tmp.x = pos.x;
    if (mov_to_leftlower(id, tmp, !reverse)) return;
    mStateMgr.changeState(StateID_FallDown);
}

void dLineMng_c::mov_frm_leftupper(const mVec2_c &pos, bool reverse) {
    u32 id = getLineUnitNo(pos.x, pos.y);
    mVec2_c tmp;
    tmp.x = pos.x;
    tmp.y = pos.y;
    tmp.x -= 16.0f;
    if (mov_to_leftlower(id, tmp, reverse)) return;
    tmp.y = pos.y;
    tmp.y += 16.0f;
    if (mov_to_leftupper(id, tmp, reverse)) return;
    tmp.x = pos.x;
    if (mov_to_rightupper(id, tmp, !reverse)) return;
    mStateMgr.changeState(StateID_FallDown);
}

// ===========================================================================
// author_core: circular-movement family
// ===========================================================================

void dLineMng_c::move_on_circle_speedset(f32 radius, f32 speedScale)
{
    f32 delta = mBaseSpeed;
    delta *= speedScale;
    mAngle = (u16)(mAngle + delta);
    mSpeed.x = mBaseSpeed * -nw4r::math::SinIdx((short)mAngle);
    mSpeed.y = mBaseSpeed * nw4r::math::CosIdx((short)mAngle);
    mPos.x = mUnk58.x;
    mPos.y = mUnk58.y;
    mPos.x += radius * nw4r::math::CosIdx((short)mAngle);
    mPos.y += radius * nw4r::math::SinIdx((short)mAngle);
}

void dLineMng_c::move_on_circle1(f32 radius, f32 speedScale)
{
    mVec2_c savedPos = mPos;
    u16 savedAngle = mAngle;
    move_on_circle_speedset(radius, speedScale);
    if (check_term()) {
        mPos.x = savedPos.x;
        mPos.y = savedPos.y;
        mAngle = savedAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    } else {
        s32 rel = (s16)mAngle;
        if (rel < 0) {
            mSpeed.x = 0.0f;
            mSpeed.y = mBaseSpeed;
            mVec2_c dst = mUnitBasePos;
            dst.x += radius;
            dst.x -= 16.0f;
            dst.y -= radius;
            dst.y += 16.0f;
            mov_frm_rightlower(dst, true);
        } else if ((s16)(rel - 0x4000) >= 0) {
            mSpeed.x = -mBaseSpeed;
            mSpeed.y = 0.0f;
            mov_frm_leftupper(mUnitBasePos, true);
        }
    }
}

void dLineMng_c::move_on_circle2(f32 radius, f32 speedScale)
{
    mVec2_c savedPos = mPos;
    u16 savedAngle = mAngle;
    move_on_circle_speedset(radius, speedScale);
    if (check_term()) {
        mPos.x = savedPos.x;
        mPos.y = savedPos.y;
        mAngle = savedAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    } else {
        s32 rel = (s16)mAngle - 0x4000;
        if ((s16)rel < 0) {
            mSpeed.x = -mBaseSpeed;
            mSpeed.y = 0.0f;
            mVec2_c dst1 = mUnitBasePos;
            dst1.x += radius;
            dst1.x -= 16.0f;
            mov_frm_rightupper(dst1, true);
        } else if ((s16)(rel - 0x4000) >= 0) {
            mSpeed.x = 0.0f;
            mSpeed.y = -mBaseSpeed;
            mVec2_c dst2 = mUnitBasePos;
            dst2.y -= radius;
            dst2.y += 16.0f;
            mov_frm_leftlower(dst2, true);
        }
    }
}

void dLineMng_c::move_on_circle3(f32 radius, f32 speedScale)
{
    mVec2_c savedPos = mPos;
    u16 savedAngle = mAngle;
    move_on_circle_speedset(radius, speedScale);
    if (check_term()) {
        mPos.x = savedPos.x;
        mPos.y = savedPos.y;
        mAngle = savedAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    } else {
        s32 rel = (s16)mAngle + 0x8000;
        if ((s16)rel < 0) {
            mSpeed.x = 0.0f;
            mSpeed.y = -mBaseSpeed;
            mov_frm_leftupper(mUnitBasePos, false);
        } else if ((s16)(rel - 0x4000) >= 0) {
            mSpeed.x = mBaseSpeed;
            mSpeed.y = 0.0f;
            mVec2_c dst = mUnitBasePos;
            dst.x += radius;
            dst.x -= 16.0f;
            dst.y -= radius;
            dst.y += 16.0f;
            mov_frm_rightlower(dst, false);
        }
    }
}

void dLineMng_c::move_on_circle4(f32 radius, f32 speedScale)
{
    mVec2_c savedPos = mPos;
    u16 savedAngle = mAngle;
    move_on_circle_speedset(radius, speedScale);
    if (check_term()) {
        mPos.x = savedPos.x;
        mPos.y = savedPos.y;
        mAngle = savedAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    } else {
        s32 rel = (s16)mAngle + 0x4000;
        if ((s16)rel < 0) {
            mSpeed.x = mBaseSpeed;
            mSpeed.y = 0.0f;
            mVec2_c dst = mUnitBasePos;
            dst.y -= radius;
            dst.y += 16.0f;
            mov_frm_leftlower(dst, false);
        } else if ((s16)(rel - 0x4000) >= 0) {
            mSpeed.x = 0.0f;
            mSpeed.y = mBaseSpeed;
            mov_frm_rightupper(mVec2_c(mUnitBasePos.x + radius - 16.0f, mUnitBasePos.y), false);
        }
    }
}

// ===========================================================================
// author_states + base skeleton: the 25 states, in .bss/.text declaration
// order. Families with real bodies are author_states; states left as trivial
// `{}` are the un-authored base skeleton (initializeState_Circle* x9 -- out
// of scope per author_states, depends on the un-authored circle_nextpos_set).
// ===========================================================================

void dLineMng_c::initializeState_Idle() {}
void dLineMng_c::finalizeState_Idle() {}
void dLineMng_c::executeState_Idle() {}

void dLineMng_c::initializeState_FallDown() {}
void dLineMng_c::finalizeState_FallDown() {}

// NOT empty in target. This is fn_800C31C0's REAL caller: a plain
// `b fn_800C31C0` tail call at the very end, confirmed at target VA
// 0x800C562C. Gravity constants read from original/wiimj2d.dol .sdata2:
// -0.0625f @ 0x8042CB90 (per-frame accel), -4.0f @ 0x8042CB94 (terminal
// velocity clamp). From wip/fix_bighelper.
void dLineMng_c::executeState_FallDown()
{
    f32 speedY = mSpeed.y;
    speedY += -0.0625f;
    mPos.x += mSpeed.x;
    if (speedY < -4.0f) {
        speedY = -4.0f;
    }
    mPos.y += speedY;
    mSpeed.y = speedY;
    fn_800C31C0(this);
}

void dLineMng_c::initializeState_Left45() {
    fn_800C3B20(this);
    mAngle = 0xE000;
    mPos.y = (mUnitBasePos.y - 16.0f) + (mPos.x - mUnitBasePos.x);
}
void dLineMng_c::finalizeState_Left45() {}
void dLineMng_c::executeState_Left45() {
    mVec2_c old = mPos;
    // Same lever as the eight mBaseSpeed*0.8910065f siblings (see
    // executeState_Left30Left), applied here even though the source used a
    // shared `dv` local rather than assigning straight to a member: the split
    // still has to land on the MEMBER (mSpeed.x), not the local, or the def-point
    // rule has nothing to anchor to. mSpeed.y is then a plain copy of mSpeed.x,
    // not a second multiply. Gets this function bit-exact.
    mSpeed.x = mBaseSpeed;
    mSpeed.x *= 0.70703f;
    mSpeed.y = mSpeed.x;
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 16.0f) + (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
        mSpeed.x *= 0.70703f;
        mSpeed.y = mSpeed.x;
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mov_frm_rightupper(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_Right45() {
    fn_800C3B20(this);
    mAngle = 0xA000;
    // SELF-ASSIGN THEN COMPOUND-SUBTRACT, not the single expression
    // `mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x)`. See the note
    // on executeState_Right45.
    mPos.y = mUnitBasePos.y;
    mPos.y -= (mPos.x - mUnitBasePos.x);
}
void dLineMng_c::finalizeState_Right45() {}
void dLineMng_c::executeState_Right45() {
    mVec2_c old = mPos;
    // Same rewrite as executeState_Left45, mSpeed.y negated instead of copied.
    // NOT SUFFICIENT ALONE here: this closed the mBaseSpeed*0.70703f inversion
    // (24 diffs down to 4) but the function still has a residual f0/f1 swap in
    // the unrelated `mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x)`
    // subtraction that this lever does not reach. Left as a strict improvement,
    // not a full match -- see b1_sweep report.
    mSpeed.x = mBaseSpeed;
    mSpeed.x *= 0.70703f;
    mSpeed.y = -mSpeed.x;
    mPos.x += mSpeed.x;
    // OPERAND EVALUATION ORDER. Written as one expression, MWCC evaluates
    // the heavier operand of the outer `-` first -- the parenthesised
    // difference -- and numbers the FP registers descending in THAT order,
    // giving mPos.x=f2 / mUnitBasePos.x=f1 / mUnitBasePos.y=f0. Retail
    // numbers them descending in SOURCE order (f2/f1/f0 over
    // mUnitBasePos.y, mPos.x, mUnitBasePos.x), which is what MWCC does once
    // the left operand has a def-point of its own ahead of the subtract.
    // Same def-point rule as lever 11, on `-` instead of `*`. A named local
    // for mUnitBasePos.y also fixes the initialize... variant but NOT this
    // one (13 diffs); the self-assign is the form that fixes both.
    mPos.y = mUnitBasePos.y;
    mPos.y -= (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
        mSpeed.x *= 0.70703f;
        mSpeed.y = -mSpeed.x;
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftupper(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mov_frm_rightlower(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_Side() {
    fn_800C3B20(this);
    mAngle = 0xC000;
    mPos.y = mUnitBasePos.y - 16.0f;
}
void dLineMng_c::finalizeState_Side() {}
void dLineMng_c::executeState_Side() {
    mVec2_c old = mPos;
    f32 baseSpeed = mBaseSpeed;
    mSpeed.x = baseSpeed;
    mSpeed.y = 0.0f;
    mPos.x += baseSpeed;
    if (check_term()) {
        mPos = old;
        f32 baseSpeed2 = mBaseSpeed;
        mSpeed.x = baseSpeed2;
        mSpeed.y = 0.0f;
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mov_frm_rightlower(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_Height() {
    fn_800C3B60(this);
    mAngle = 0x0;
    // LEFT-OPERAND DEF-POINT (lever 11's rule 1, generalised off `*`).
    // Written as one expression MWCC evaluates the HEAVIER operand of the
    // outer `+` first and numbers the FP registers descending in THAT order;
    // retail numbers them descending in SOURCE order. Giving the light left
    // operand a def-point of its own ahead of the operator restores source
    // order. Same fix as executeState_Right45.
    mPos.x = mUnitBasePos.x;
    mPos.x += 16.0f;
}
void dLineMng_c::finalizeState_Height() {}
void dLineMng_c::executeState_Height() {
    mVec2_c old = mPos;
    f32 baseSpeed = mBaseSpeed;
    mSpeed.x = 0.0f;
    mSpeed.y = baseSpeed;
    mPos.y = mPos.y + baseSpeed;
    if (check_term()) {
        mPos = old;
        f32 baseSpeed2 = mBaseSpeed;
        mSpeed.x = 0.0f;
        mSpeed.y = baseSpeed2;
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mov_frm_rightlower(mUnitBasePos, true);
    } else if (mPos.y >= mUnitBasePos.y) {
        mov_frm_rightupper(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_CornerHeightLine() {
    fn_800C3B60(this);
    mAngle = 0x0;
    // LEFT-OPERAND DEF-POINT (lever 11's rule 1, generalised off `*`).
    // Written as one expression MWCC evaluates the HEAVIER operand of the
    // outer `+` first and numbers the FP registers descending in THAT order;
    // retail numbers them descending in SOURCE order. Giving the light left
    // operand a def-point of its own ahead of the operator restores source
    // order. Same fix as executeState_Right45.
    mPos.x = mUnitBasePos.x;
    mPos.x += 16.0f;
}
void dLineMng_c::finalizeState_CornerHeightLine() {}
void dLineMng_c::executeState_CornerHeightLine() {
    mVec2_c old = mPos;
    f32 baseSpeed = mBaseSpeed;
    mSpeed.x = 0.0f;
    mSpeed.y = baseSpeed;
    mPos.y = mPos.y + baseSpeed;
    if (check_term()) {
        mPos = old;
        f32 baseSpeed2 = mBaseSpeed;
        mSpeed.x = 0.0f;
        mSpeed.y = baseSpeed2;
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mStateMgr.changeState(StateID_CornerSideLine);
    } else if (mPos.y >= mUnitBasePos.y) {
        mov_frm_rightupper(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_CornerSideLine() {
    fn_800C3B20(this);
    mAngle = 0xC000;
    mPos.y = mUnitBasePos.y - 16.0f;
}
void dLineMng_c::finalizeState_CornerSideLine() {}
void dLineMng_c::executeState_CornerSideLine() {
    mVec2_c old = mPos;
    f32 baseSpeed = mBaseSpeed;
    mSpeed.x = baseSpeed;
    mSpeed.y = 0.0f;
    mPos.x += baseSpeed;
    if (check_term()) {
        mPos = old;
        f32 baseSpeed2 = mBaseSpeed;
        mSpeed.x = baseSpeed2;
        mSpeed.y = 0.0f;
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mStateMgr.changeState(StateID_CornerHeightLine);
    }
}

void dLineMng_c::initializeState_Left30Left() {
    fn_800C3B20(this);
    mAngle = 0xD333;
    mPos.y = (mUnitBasePos.y - 16.0f) + 0.5 * (mPos.x - mUnitBasePos.x);
}
void dLineMng_c::finalizeState_Left30Left() {}
void dLineMng_c::executeState_Left30Left() {
    mVec2_c old = mPos;
    // SPLIT ASSIGNMENT, not `mSpeed.x = mBaseSpeed * 0.8910065f;`. Two separate
    // rules are at work and the split satisfies both at once.
    //   1. REGISTER CHOICE: a multiply's variable operand only lands in f1 if it
    //      has a def-point of its own ahead of the multiply. Written as one
    //      expression the member has no def and lands in f0, so the pair comes
    //      out permuted against retail. Any separate def fixes this -- a short
    //      local works too -- but a local hoisted to function top is worse (+3,
    //      it forces the callee-saved f31).
    //   2. fmuls SLOT ORDER: MWCC canonically puts the literal operand in the
    //      FIRST source slot regardless of how the source is written, which is
    //      why swapping the operands compiles byte-identical. `x *= k` escapes
    //      the rule entirely because the destination IS the first operand.
    // Applies ONLY to the mBaseSpeed products. Extending the same form to the
    // half-products (`mSpeed.y = 0.5f * mSpeed.x`) is MEASURED strictly worse:
    // it breaks Left30Left and destroys Right30Right and Right60Up.
    // See "Gap B" in HANDOFF.md.
    mSpeed.x = mBaseSpeed;
    mSpeed.x *= 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 16.0f) + 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
        mSpeed.x *= 0.8910065f;
        mSpeed.y = 0.5f * mSpeed.x;
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        // COPY-THEN-ADJUST, not `mVec2_c newBase(mUnitBasePos.x + 16.0f,
        // mUnitBasePos.y)`. The constructor form lets -O4 reuse the sum it
        // just computed for the `>=` test one line above; the target instead
        // reloads mUnitBasePos.x and re-adds. An aggregate copy's field loads
        // are not CSE'd against earlier scalar reads of the same members, so
        // this form reproduces the reload, the re-add AND the y-before-x store
        // order exactly. All eight executeState_* siblings share it.
        // See "Gap A" in HANDOFF.md.
        mVec2_c newBase = mUnitBasePos;
        newBase.x += 16.0f;
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

void dLineMng_c::initializeState_Left30Right() {
    fn_800C3B20(this);
    mAngle = 0xD333;
    mPos.y = (mUnitBasePos.y - 8.0f) + 0.5 * (mPos.x - mUnitBasePos.x);
}
void dLineMng_c::finalizeState_Left30Right() {}
void dLineMng_c::executeState_Left30Right() {
    mVec2_c old = mPos;
    mSpeed.x = mBaseSpeed;
    mSpeed.x *= 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 8.0f) + 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
        mSpeed.x *= 0.8910065f;
        // DIVISION, not `0.5f * mSpeed.x`, at THIS occurrence only -- the head
        // site above keeps the multiply. Retail really is asymmetric here:
        // this restore block is the ONLY one of the five sibling states whose
        // half-product has the member in fmuls slot A (`fmuls f0, f1, f0`);
        // Left30Left, Right30Left, Right30Right and Right60Up all emit
        // `fmuls f0, f0, f1` here and match with the plain multiply. Both
        // operands are already live, so lever 11's load-order rule cannot
        // reach it; only a source route that never shows the canonicaliser a
        // syntactic literal does. `/ 2.0f` (reciprocal-multiply route) and
        // `mSpeed.y = mSpeed.x; mSpeed.y *= 0.5f;` (compound route) compile
        // byte-identical; using `/ 2.0f` to keep the block a 3-statement
        // parallel of its siblings. Applying it at BOTH sites just moves the
        // defect to the head multiply (idx 19), so the asymmetry is forced.
        mSpeed.y = mSpeed.x / 2.0f;
    } else if (mPos.x < mUnitBasePos.x) {
        mVec2_c newBase = mUnitBasePos;
        newBase.x -= 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 9) {
            mUnitBasePos.x = newBase.x;
            mStateMgr.changeState(StateID_Left30Left);
        } else if (lineUnitNo == 0xB) {
            mUnitBasePos.x = newBase.x;
            mStateMgr.changeState(StateID_Right30Left);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mov_frm_rightupper(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_Right30Left() {
    fn_800C3B20(this);
    mAngle = 0xACCC;
    mPos.y = mUnitBasePos.y - 0.5 * (mPos.x - mUnitBasePos.x);
}
void dLineMng_c::finalizeState_Right30Left() {}
void dLineMng_c::executeState_Right30Left() {
    mVec2_c old = mPos;
    mSpeed.x = mBaseSpeed;
    mSpeed.x *= 0.8910065f;
    mSpeed.y = -(0.5f * mSpeed.x);
    mPos.x += mSpeed.x;
    // LEFT-OPERAND DEF-POINT (lever 11's rule 1, generalised off `*`).
    // Written as one expression MWCC evaluates the HEAVIER operand of the
    // outer +/- first and numbers the FP registers descending in THAT order;
    // retail numbers them descending in SOURCE order. Giving the light left
    // operand a def-point of its own ahead of the operator restores source
    // order. Same fix as executeState_Right45.
    mPos.y = mUnitBasePos.y;
    mPos.y -= 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
        mSpeed.x *= 0.8910065f;
        mSpeed.y = -(0.5f * mSpeed.x);
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftupper(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mVec2_c newBase = mUnitBasePos;
        newBase.x += 16.0f;
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

void dLineMng_c::initializeState_Right30Right() {
    fn_800C3B20(this);
    mAngle = 0xACCC;
    mPos.y = (mUnitBasePos.y - 8.0f) - 0.5f * (mPos.x - mUnitBasePos.x);
}
void dLineMng_c::finalizeState_Right30Right() {}
void dLineMng_c::executeState_Right30Right() {
    mVec2_c old = mPos;
    mSpeed.x = mBaseSpeed;
    mSpeed.x *= 0.8910065f;
    mSpeed.y = -(0.5f * mSpeed.x);
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 8.0f) - 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
        mSpeed.x *= 0.8910065f;
        mSpeed.y = -(0.5f * mSpeed.x);
    } else if (mPos.x < mUnitBasePos.x) {
        mVec2_c newBase = mUnitBasePos;
        newBase.x -= 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 9) {
            mUnitBasePos.x = newBase.x;
            mStateMgr.changeState(StateID_Left30Left);
        } else if (lineUnitNo == 0xB) {
            mUnitBasePos = newBase;
            mStateMgr.changeState(StateID_Right30Left);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mov_frm_rightlower(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_Left60Up() {
    fn_800C3B60(this);
    mAngle = 0xECCC;
    // LEFT-OPERAND DEF-POINT (lever 11's rule 1, generalised off `*`).
    // Written as one expression MWCC evaluates the HEAVIER operand of the
    // outer +/- first and numbers the FP registers descending in THAT order;
    // retail numbers them descending in SOURCE order. Giving the light left
    // operand a def-point of its own ahead of the operator restores source
    // order. Same fix as executeState_Right45.
    // Both operators need it here: the inner `+` and the outer `-`.
    mPos.x = mUnitBasePos.x;
    mPos.x += 16.0f;
    mPos.x -= 0.5 * (mUnitBasePos.y - mPos.y);
}
void dLineMng_c::finalizeState_Left60Up() {}
void dLineMng_c::executeState_Left60Up() {
    mVec2_c old = mPos;
    mSpeed.y = mBaseSpeed;
    mSpeed.y *= 0.8910065f;
    mSpeed.x = 0.5f * mSpeed.y;
    mPos.y += mSpeed.y;
    // LEFT-OPERAND DEF-POINT (lever 11's rule 1, generalised off `*`).
    // Written as one expression MWCC evaluates the HEAVIER operand of the
    // outer +/- first and numbers the FP registers descending in THAT order;
    // retail numbers them descending in SOURCE order. Giving the light left
    // operand a def-point of its own ahead of the operator restores source
    // order. Same fix as executeState_Right45.
    // Both operators need it here: the inner `+` and the outer `-`.
    mPos.x = mUnitBasePos.x;
    mPos.x += 16.0f;
    mPos.x -= 0.5 * (mUnitBasePos.y - mPos.y);
    if (check_term()) {
        mPos = old;
        mSpeed.y = mBaseSpeed;
        mSpeed.y *= 0.8910065f;
        mSpeed.x = 0.5f * mSpeed.y;
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mVec2_c newBase = mUnitBasePos;
        newBase.y -= 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 0xD) {
            mUnitBasePos.y = newBase.y;
            mStateMgr.changeState(StateID_Left60Down);
        } else if (lineUnitNo == 0xE) {
            mUnitBasePos.y = newBase.y;
            mStateMgr.changeState(StateID_Right60Down);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    } else if (mPos.y >= mUnitBasePos.y) {
        mov_frm_rightupper(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_Left60Down() {
    fn_800C3B60(this);
    mAngle = 0xECCC;
    mPos.x = mUnitBasePos.x;
    mPos.x += 8.0;
    mPos.x -= 0.5 * (mUnitBasePos.y - mPos.y);
}
void dLineMng_c::finalizeState_Left60Down() {}
void dLineMng_c::executeState_Left60Down() {
    mVec2_c old = mPos;
    mSpeed.y = mBaseSpeed;
    mSpeed.y *= 0.8910065f;
    mSpeed.x = 0.5f * mSpeed.y;
    mPos.y += mSpeed.y;
    // DOUBLE-PRECISION SPLIT ASSIGNMENT. `8.0` and `0.5` are unsuffixed, so
    // both are DOUBLES -- confirmed by decoding the pool: retail loads them
    // with `lfd` from 0x8042CBB0 (4020000000000000 = 8.0) and 0x8042CBA0
    // (3FE0000000000000 = 0.5), while the float 0.5f/16.0f used elsewhere in
    // this file sit separately at 0x8042CB5C/0x8042CB48 as 4-byte floats.
    // The shape is identical to the MATCHING executeState_Left60Up two
    // functions up (`mPos.x = mUnitBasePos.x; mPos.x += 16.0f; mPos.x -= ...`).
    // Lever 11 DOES extend to double precision: the compound assignment puts
    // the member in the FIRST source slot (`fadd f2, f3, f2`), where the
    // combined form `mUnitBasePos.x + 8.0` hoists the literal there instead.
    // The intermediate `frsp` is the rounding the float member's `+=` requires
    // -- Left60Up needs none only because `fadds` already rounds.
    mPos.x = mUnitBasePos.x;
    mPos.x += 8.0;
    mPos.x -= 0.5 * (mUnitBasePos.y - mPos.y);
    if (check_term()) {
        mPos = old;
        mSpeed.y = mBaseSpeed;
        mSpeed.y *= 0.8910065f;
        mSpeed.x = 0.5f * mSpeed.y;
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.y >= mUnitBasePos.y) {
        mVec2_c newBase = mUnitBasePos;
        newBase.y += 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 0xC) {
            mUnitBasePos.y = newBase.y;
            mStateMgr.changeState(StateID_Left60Up);
        } else if (lineUnitNo == 0xF) {
            mUnitBasePos.y = newBase.y;
            mStateMgr.changeState(StateID_Right60Up);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    }
}

void dLineMng_c::initializeState_Right60Down() {
    fn_800C3B60(this);
    mAngle = 0x9333;
    mPos.x = mUnitBasePos.x;
    mPos.x += 8.0;
    mPos.x += 0.5 * (mUnitBasePos.y - mPos.y);
}
void dLineMng_c::finalizeState_Right60Down() {}
void dLineMng_c::executeState_Right60Down() {
    mVec2_c old = mPos;
    mSpeed.y = mBaseSpeed;
    mSpeed.y *= -0.8910065f;
    mSpeed.x = -(0.5f * mSpeed.y);
    mPos.y += mSpeed.y;
    // See executeState_Left60Down for the double-precision reasoning; this is
    // the same statement with the outer sign flipped.
    mPos.x = mUnitBasePos.x;
    mPos.x += 8.0;
    mPos.x += 0.5 * (mUnitBasePos.y - mPos.y);
    if (check_term()) {
        mPos = old;
        mSpeed.y = mBaseSpeed;
        mSpeed.y *= -0.8910065f;
        mSpeed.x = -(0.5f * mSpeed.y);
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mov_frm_rightlower(mUnitBasePos, false);
    } else if (mPos.y >= mUnitBasePos.y) {
        mVec2_c newBase = mUnitBasePos;
        newBase.y += 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 0xC) {
            mUnitBasePos.y = newBase.y;
            mStateMgr.changeState(StateID_Left60Up);
        } else if (lineUnitNo == 0xF) {
            mUnitBasePos.y = newBase.y;
            mStateMgr.changeState(StateID_Right60Up);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    }
}

void dLineMng_c::initializeState_Right60Up() {
    fn_800C3B60(this);
    mAngle = 0x9333;
    mPos.x = mUnitBasePos.x + 0.5 * (mUnitBasePos.y - mPos.y);
}
void dLineMng_c::finalizeState_Right60Up() {}
void dLineMng_c::executeState_Right60Up() {
    mVec2_c old = mPos;
    mSpeed.y = mBaseSpeed;
    mSpeed.y *= -0.8910065f;
    mSpeed.x = -(0.5f * mSpeed.y);
    mPos.y += mSpeed.y;
    mPos.x = mUnitBasePos.x + 0.5 * (mUnitBasePos.y - mPos.y);
    if (check_term()) {
        mPos = old;
        mSpeed.y = mBaseSpeed;
        mSpeed.y *= -0.8910065f;
        mSpeed.x = -(0.5f * mSpeed.y);
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mVec2_c newBase = mUnitBasePos;
        newBase.y -= 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 0xD) {
            mUnitBasePos.y = newBase.y;
            mStateMgr.changeState(StateID_Left60Down);
        } else if (lineUnitNo == 0xE) {
            mUnitBasePos.y = newBase.y;
            mStateMgr.changeState(StateID_Right60Down);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    } else if (mPos.y >= mUnitBasePos.y) {
        mov_frm_leftupper(mUnitBasePos, false);
    }
}

// The nine Circle initialisers are one family and the retail constants are
// perfectly regular once decoded out of the DOL's .sdata2 rather than guessed
// from the instruction pattern -- every `lfs ...@sda21(r0)` here has its offset
// field ZEROED in both disassemblies, so a wrong constant still compares
// byte-identical. Decoded values:  Left => vec.x = +size, Right => vec.x = 0;
//                                  Up   => vec.y = -size, Down  => vec.y = 0.
// The radius argument is always +size.
//
// That last line is what makes the word counts uneven, and it is not a source
// difference. The radius travels in f1, and so does vec.x. When they are equal
// (every Left variant) one `lfs` serves both and the function is 13 words. When
// vec.x is 0 and the radius is not (the *up variants) f1 must be loaded twice,
// giving 14. RightDown is 13 again because 0.0f fills BOTH vector slots from a
// single load. Same source shape throughout -- only the operands differ.
void dLineMng_c::initializeState_Circle() {
    circle_nextpos_set(mVec2_c(8.0f, -8.0f), 8.0f);
}
void dLineMng_c::finalizeState_Circle() {}
void dLineMng_c::executeState_Circle() {
    mVec2_c old = mPos;
    u16 oldAngle = mAngle;
    move_on_circle_speedset(8.0f, 1303.79833984375f);
    if (check_term()) {
        mPos = old;
        mAngle = oldAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    }
}

void dLineMng_c::initializeState_Circle2x2Leftup() {
    circle_nextpos_set(mVec2_c(16.0f, -16.0f), 16.0f);
}
void dLineMng_c::finalizeState_Circle2x2Leftup() {}
void dLineMng_c::executeState_Circle2x2Leftup() {
    move_on_circle2(16.0f, 651.899169921875f);
}

void dLineMng_c::initializeState_Circle2x2Rightup() {
    circle_nextpos_set(mVec2_c(0.0f, -16.0f), 16.0f);
}
void dLineMng_c::finalizeState_Circle2x2Rightup() {}
void dLineMng_c::executeState_Circle2x2Rightup() {
    move_on_circle1(16.0f, 651.899169921875f);
}

void dLineMng_c::initializeState_Circle2x2LeftDown() {
    circle_nextpos_set(mVec2_c(16.0f, 0.0f), 16.0f);
}
void dLineMng_c::finalizeState_Circle2x2LeftDown() {}
void dLineMng_c::executeState_Circle2x2LeftDown() {
    move_on_circle3(16.0f, 651.899169921875f);
}

void dLineMng_c::initializeState_Circle2x2RightDown() {
    circle_nextpos_set(mVec2_c(0.0f, 0.0f), 16.0f);
}
void dLineMng_c::finalizeState_Circle2x2RightDown() {}
void dLineMng_c::executeState_Circle2x2RightDown() {
    move_on_circle4(16.0f, 651.899169921875f);
}

void dLineMng_c::initializeState_Circle4x4Rightup() {
    circle_nextpos_set(mVec2_c(0.0f, -32.0f), 32.0f);
}
void dLineMng_c::finalizeState_Circle4x4Rightup() {}
void dLineMng_c::executeState_Circle4x4Rightup() {
    move_on_circle1(32.0f, 325.9495849609375f);
}

void dLineMng_c::initializeState_Circle4x4LeftUp() {
    circle_nextpos_set(mVec2_c(32.0f, -32.0f), 32.0f);
}
void dLineMng_c::finalizeState_Circle4x4LeftUp() {}
void dLineMng_c::executeState_Circle4x4LeftUp() {
    move_on_circle2(32.0f, 325.9495849609375f);
}

void dLineMng_c::initializeState_Circle4x4LeftDown() {
    circle_nextpos_set(mVec2_c(32.0f, 0.0f), 32.0f);
}
void dLineMng_c::finalizeState_Circle4x4LeftDown() {}
void dLineMng_c::executeState_Circle4x4LeftDown() {
    move_on_circle3(32.0f, 325.9495849609375f);
}

void dLineMng_c::initializeState_Circle4x4RightDown() {
    circle_nextpos_set(mVec2_c(0.0f, 0.0f), 32.0f);
}
void dLineMng_c::finalizeState_Circle4x4RightDown() {}
void dLineMng_c::executeState_Circle4x4RightDown() {
    move_on_circle4(32.0f, 325.9495849609375f);
}
