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

void dLineMng_c::SetPos(const mVec2_c &pos)
{
    mPos.x = pos.x;
    mPos.y = pos.y;
}

f32 dLineMng_c::CalcAdjustPosY(f32 a, f32 b)
{
    f32 origSpeed = mBaseSpeed;
    f32 x = GetPos().x;
    f32 absB = std::fabs(b);
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
    if (t > -0.1 && t < 0.1) {
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
static u8 s_dDirInit = 0;

void dLineMng_c::init_term_ck_pos()
{
    mVec2_c *p = mDirVec;
    do {
        p->x = 0.0f;
        p->y = 0.0f;
        p++;
    } while (p != mDirVec + 3);

    if (!s_dDirInit) {
        s_dDir[0].set(-0.1f, 0.1f);
        s_dDir[1].set(0.1f, 0.1f);
        s_dDir[2].set(-0.1f, -0.1f);
        s_dDir[3].set(0.1f, -0.1f);
        s_dDirInit = 1;
    }

    const mVec2_POD_c *q = s_dDir;
    do {
        p->x = q->x;
        p->y = q->y;
        q++;
        p++;
    } while (p != mDirVec + 7);
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
        // Y computed before X -- target's load/fadds/fdivs order processes
        // the Y component first. From wip/fix_bighelper.
        mVec2_c testPos;
        testPos.y = (f32)(int)((mPos.y + p->y) / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X;
        testPos.x = (f32)(int)((mPos.x + p->x) / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X;
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

        if (!(out.x >= -0.1f && out.x <= p2 + 0.1f)) {
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

// @unofficial unnamed file-scope helper (target `fn_800C1EE0`, author_geom).
// Reconstructed from width_cross_chk's caller side and its own body: takes a
// dLineMng_c* that passes straight through from the caller's own `this`
// (unmodified before the call), so it is a free function, not a class member
// (member statics in this unit -- line_cross_chk1 etc. -- carry real mangled
// names in the target; this one has none at all). Needs `friend` access to
// touch mPos/mUnitBasePos -- see the MERGE-LOCAL header addition.
bool fn_800C1EE0(dLineMng_c *pThis, f32 a, f32 b, const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3, const mVec2_c &origin) {
    mVec2_c out;
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
    mVec2_c posOld;
    posOld.x = (f32)(int)self->mOldPos.x;
    posOld.y = (f32)(int)self->mOldPos.y;

    mVec2_c posNew;
    posNew.x = (f32)(int)self->mPos.x;
    posNew.y = (f32)(int)self->mPos.y;

    // Grid-snap mPos down to its UNIT_SIZE cell, then step back one more
    // cell so the 3x3 scan below is centred on the unit mPos sits in.
    // `base` is initialised by AGGREGATE COPY and then rewritten field by
    // field, rather than being filled from `self->mPos.x`/`self->mPos.y`
    // directly. That is not a stylistic choice: MEASURED, the field loads
    // MWCC emits for an aggregate copy are NOT common-subexpression-eliminated
    // against the earlier scalar reads of the same members in posNew's
    // construction above, so both fields are genuinely reloaded -- which is
    // what the target does. Written the direct way, -O4 reuses the live
    // registers and the function lands 2 words short (1 per field, confirmed
    // additively). See "Gap A" in HANDOFF.md.
    //
    // An earlier round forced the same reload with `*(volatile f32 *)&...` and
    // that was correctly rejected as inauthentic. The two forms compile to
    // BYTE-IDENTICAL code, so nothing is lost by preferring this one.
    mVec2_c base = self->mPos;
    base.x = dLineMng_c::smc_UNIT_SIZE_X * (f32)(int)(base.x / dLineMng_c::smc_UNIT_SIZE_X) - 16.0f;
    base.y = dLineMng_c::smc_UNIT_SIZE_X * (f32)(int)(base.y / dLineMng_c::smc_UNIT_SIZE_X) - 16.0f;

    mVec2_c pos;
    mVec2_c corner;
    pos.x = base.x;
    pos.y = base.y;
    for (int j = 0; j < 3; j++) {
        pos.x = base.x;
        for (int i = 0; i < 3; i++) {
            u32 id = dLineMng_c::getLineUnitNo(pos.x, pos.y);
            switch (id) {
                case 1:
                    if (self->line0_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 2:
                    if (self->line1_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 4:
                    if (self->line3h_cross_chk(pos, posOld, posNew) ||
                        self->line3v_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 5:
                    if (self->line4_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 6:
                    if (self->line5_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 9:
                    if (self->line7_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 8:
                    if (self->line8_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 11:
                    if (self->line9_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 10:
                    if (self->lineA_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 12:
                    if (self->lineB_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 13:
                    if (self->lineC_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 14:
                    if (self->lineD_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 15:
                    if (self->lineE_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 16:
                    if (self->lineF_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 18:
                    if (self->circle_ul2_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 17:
                    if (self->circle_ur2_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 20:
                    if (self->circle_dl2_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 19:
                    if (self->circle_dr2_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 26:
                    corner.x = pos.x - 16.0f;
                    corner.y = pos.y + 16.0f;
                    if (self->lineRHUR_cross_chk(corner, posOld, posNew)) goto found;
                    break;
                case 24:
                    corner.x = pos.x - 16.0f;
                    corner.y = pos.y;
                    if (self->lineRHUR_cross_chk(corner, posOld, posNew)) goto found;
                    break;
                case 23:
                    if (self->lineRHUR_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 22:
                    corner.x = pos.x - 16.0f;
                    corner.y = pos.y;
                    if (self->lineRHUL_cross_chk(corner, posOld, posNew)) goto found;
                    break;
                case 21:
                    if (self->lineRHUL_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 25:
                    corner.x = pos.x;
                    corner.y = pos.y + 16.0f;
                    if (self->lineRHUL_cross_chk(corner, posOld, posNew)) goto found;
                    break;
                case 27:
                    if (self->lineRHLL_cross_chk(pos, posOld, posNew)) goto found;
                    break;
                case 29:
                    corner.x = pos.x;
                    corner.y = pos.y + 16.0f;
                    if (self->lineRHLL_cross_chk(corner, posOld, posNew)) goto found;
                    break;
                case 30:
                    corner.x = pos.x - 16.0f;
                    corner.y = pos.y + 16.0f;
                    if (self->lineRHLL_cross_chk(corner, posOld, posNew)) goto found;
                    break;
                case 31:
                    corner.x = pos.x;
                    corner.y = pos.y + 16.0f;
                    if (self->lineRHLR_cross_chk(corner, posOld, posNew)) goto found;
                    break;
                case 32:
                    corner.x = pos.x - 16.0f;
                    corner.y = pos.y + 16.0f;
                    if (self->lineRHLR_cross_chk(corner, posOld, posNew)) goto found;
                    break;
                case 28:
                    corner.x = pos.x - 16.0f;
                    corner.y = pos.y;
                    if (self->lineRHLR_cross_chk(corner, posOld, posNew)) goto found;
                    break;
            }
            pos.x += 16.0f;
        }
        pos.y += 16.0f;
    }
found:
    return;
}

// ===========================================================================
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
    if (self->mUnitBasePos.x > self->mPos.x) {
        self->mPos.x = self->mUnitBasePos.x;
    }
    f32 upper = self->mUnitBasePos.x + 16.0f;
    if (self->mPos.x < upper) {
        return;
    }
    self->mPos.x = upper - 0.1f;
}

/// @unofficial fn_800C3B60 (0x3C bytes, 15 words). Mirrors fn_800C3B20 for
/// the Y axis.
static void fn_800C3B60(dLineMng_c *self)
{
    if (self->mPos.y >= self->mUnitBasePos.y) {
        self->mPos.y = self->mUnitBasePos.y - 0.1f;
    }
    f32 lower = self->mUnitBasePos.y - 16.0f;
    if (!(self->mPos.y < lower)) {
        return;
    }
    self->mPos.y = lower;
}

// ===========================================================================
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
    mAngle = (u16)(mAngle + mBaseSpeed * speedScale);
    mSpeed.x = -mBaseSpeed * nw4r::math::SinIdx((short)mAngle);
    mSpeed.y = mBaseSpeed * nw4r::math::CosIdx((short)mAngle);
    mPos.x = mUnk58.x;
    mPos.y = mUnk58.y;
    mPos.x += radius * nw4r::math::CosIdx((short)mAngle);
    mPos.y += radius * nw4r::math::SinIdx((short)mAngle);
}

void dLineMng_c::move_on_circle1(f32 radius, f32 speedScale)
{
    mVec2_c savedPos;
    savedPos.x = mPos.x;
    savedPos.y = mPos.y;
    u16 savedAngle = mAngle;
    move_on_circle_speedset(radius, speedScale);
    if (check_term()) {
        mPos.x = savedPos.x;
        mPos.y = savedPos.y;
        mAngle = savedAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    } else if ((s16)mAngle < 0) {
        mVec2_c dst;
        dst.x = mUnitBasePos.x + radius - 16.0f;
        dst.y = mUnitBasePos.y - radius + 16.0f;
        mSpeed.x = 0.0f;
        mSpeed.y = mBaseSpeed;
        mov_frm_rightlower(dst, true);
    } else if ((s16)(mAngle - 0x4000) >= 0) {
        mSpeed.x = -mBaseSpeed;
        mSpeed.y = 0.0f;
        mov_frm_leftupper(mUnitBasePos, true);
    }
}

void dLineMng_c::move_on_circle2(f32 radius, f32 speedScale)
{
    mVec2_c savedPos;
    savedPos.x = mPos.x;
    savedPos.y = mPos.y;
    u16 savedAngle = mAngle;
    move_on_circle_speedset(radius, speedScale);
    if (check_term()) {
        mPos.x = savedPos.x;
        mPos.y = savedPos.y;
        mAngle = savedAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    } else {
        s16 rel = mAngle - 0x4000;
        if (rel < 0) {
            mVec2_c dst;
            dst.x = mUnitBasePos.x + radius - 16.0f;
            dst.y = mUnitBasePos.y;
            mSpeed.x = -mBaseSpeed;
            mSpeed.y = 0.0f;
            mov_frm_rightupper(dst, true);
        } else if ((s16)(rel - 0x4000) >= 0) {
            mVec2_c dst;
            dst.x = mUnitBasePos.x;
            dst.y = mUnitBasePos.y - radius + 16.0f;
            mSpeed.x = 0.0f;
            mSpeed.y = -mBaseSpeed;
            mov_frm_leftlower(dst, true);
        }
    }
}

void dLineMng_c::move_on_circle3(f32 radius, f32 speedScale)
{
    mVec2_c savedPos;
    savedPos.x = mPos.x;
    savedPos.y = mPos.y;
    u16 savedAngle = mAngle;
    move_on_circle_speedset(radius, speedScale);
    if (check_term()) {
        mPos.x = savedPos.x;
        mPos.y = savedPos.y;
        mAngle = savedAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    } else {
        s16 rel = mAngle + 0x8000;
        if (rel < 0) {
            mSpeed.x = 0.0f;
            mSpeed.y = -mBaseSpeed;
            mov_frm_leftupper(mUnitBasePos, false);
        } else if ((s16)(rel - 0x4000) >= 0) {
            mVec2_c dst;
            dst.x = mUnitBasePos.x + radius - 16.0f;
            dst.y = mUnitBasePos.y - radius + 16.0f;
            mSpeed.x = mBaseSpeed;
            mSpeed.y = 0.0f;
            mov_frm_rightlower(dst, false);
        }
    }
}

void dLineMng_c::move_on_circle4(f32 radius, f32 speedScale)
{
    mVec2_c savedPos;
    savedPos.x = mPos.x;
    savedPos.y = mPos.y;
    u16 savedAngle = mAngle;
    move_on_circle_speedset(radius, speedScale);
    if (check_term()) {
        mPos.x = savedPos.x;
        mPos.y = savedPos.y;
        mAngle = savedAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    } else {
        s16 rel = mAngle + 0x4000;
        if (rel < 0) {
            mVec2_c dst;
            dst.x = mUnitBasePos.x;
            dst.y = mUnitBasePos.y - radius + 16.0f;
            mSpeed.x = mBaseSpeed;
            mSpeed.y = 0.0f;
            mov_frm_leftlower(dst, false);
        } else if ((s16)(rel - 0x4000) >= 0) {
            mVec2_c dst;
            dst.x = mUnitBasePos.x + radius - 16.0f;
            dst.y = mUnitBasePos.y;
            mSpeed.x = 0.0f;
            mSpeed.y = mBaseSpeed;
            mov_frm_rightupper(dst, false);
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
    f32 speedY = mSpeed.y + -0.0625f;
    mPos.x += mSpeed.x;
    if (speedY < -4.0f) {
        speedY = -4.0f;
    }
    mSpeed.y = speedY;
    mPos.y += mSpeed.y;
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
    mPos.y = mUnitBasePos.y;
    mPos.y -= mPos.x - mUnitBasePos.x;
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
    mPos.y = mUnitBasePos.y;
    mPos.y -= mPos.x - mUnitBasePos.x;
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
    mSpeed.x = mBaseSpeed;
    mSpeed.y = 0.0f;
    mPos.x += mBaseSpeed;
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
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
    mPos.x = mUnitBasePos.x + 16.0f;
}
void dLineMng_c::finalizeState_Height() {}
void dLineMng_c::executeState_Height() {
    mVec2_c old = mPos;
    mSpeed.x = 0.0f;
    mSpeed.y = mBaseSpeed;
    mPos.y = mPos.y + mBaseSpeed;
    if (check_term()) {
        mPos = old;
        mSpeed.x = 0.0f;
        mSpeed.y = mBaseSpeed;
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mov_frm_rightlower(mUnitBasePos, true);
    } else if (mPos.y >= mUnitBasePos.y) {
        mov_frm_rightupper(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_CornerHeightLine() {
    fn_800C3B60(this);
    mAngle = 0x0;
    mPos.x = mUnitBasePos.x + 16.0f;
}
void dLineMng_c::finalizeState_CornerHeightLine() {}
void dLineMng_c::executeState_CornerHeightLine() {
    mVec2_c old = mPos;
    mSpeed.x = 0.0f;
    mSpeed.y = mBaseSpeed;
    mPos.y = mPos.y + mBaseSpeed;
    if (check_term()) {
        mPos = old;
        mSpeed.x = 0.0f;
        mSpeed.y = mBaseSpeed;
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
    mSpeed.x = mBaseSpeed;
    mSpeed.y = 0.0f;
    mPos.x += mBaseSpeed;
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
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
        mSpeed.y = 0.5f * mSpeed.x;
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
    mPos.y = mUnitBasePos.y - 0.5f * (mPos.x - mUnitBasePos.x);
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
    mPos.x = (mUnitBasePos.x + 16.0f) - 0.5 * (mUnitBasePos.y - mPos.y);
}
void dLineMng_c::finalizeState_Left60Up() {}
void dLineMng_c::executeState_Left60Up() {
    mVec2_c old = mPos;
    mSpeed.y = mBaseSpeed;
    mSpeed.y *= 0.8910065f;
    mSpeed.x = 0.5f * mSpeed.y;
    mPos.y += mSpeed.y;
    mPos.x = (mUnitBasePos.x + 16.0f) - 0.5 * (mUnitBasePos.y - mPos.y);
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
    f32 t = mUnitBasePos.x + 8.0;
    mPos.x = t - 0.5 * (mUnitBasePos.y - mPos.y);
}
void dLineMng_c::finalizeState_Left60Down() {}
void dLineMng_c::executeState_Left60Down() {
    mVec2_c old = mPos;
    mSpeed.y = mBaseSpeed;
    mSpeed.y *= 0.8910065f;
    mSpeed.x = 0.5f * mSpeed.y;
    mPos.y += mSpeed.y;
    f32 t = mUnitBasePos.x + 8.0;
    mPos.x = t - 0.5 * (mUnitBasePos.y - mPos.y);
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
    f32 t = mUnitBasePos.x + 8.0;
    mPos.x = t + 0.5 * (mUnitBasePos.y - mPos.y);
}
void dLineMng_c::finalizeState_Right60Down() {}
void dLineMng_c::executeState_Right60Down() {
    mVec2_c old = mPos;
    mSpeed.y = mBaseSpeed;
    mSpeed.y *= -0.8910065f;
    mSpeed.x = -(0.5f * mSpeed.y);
    mPos.y += mSpeed.y;
    f32 t = mUnitBasePos.x + 8.0;
    mPos.x = t + 0.5 * (mUnitBasePos.y - mPos.y);
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

void dLineMng_c::initializeState_Circle() {}
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

void dLineMng_c::initializeState_Circle2x2Leftup() {}
void dLineMng_c::finalizeState_Circle2x2Leftup() {}
void dLineMng_c::executeState_Circle2x2Leftup() {
    move_on_circle2(16.0f, 651.899169921875f);
}

void dLineMng_c::initializeState_Circle2x2Rightup() {}
void dLineMng_c::finalizeState_Circle2x2Rightup() {}
void dLineMng_c::executeState_Circle2x2Rightup() {
    move_on_circle1(16.0f, 651.899169921875f);
}

void dLineMng_c::initializeState_Circle2x2LeftDown() {}
void dLineMng_c::finalizeState_Circle2x2LeftDown() {}
void dLineMng_c::executeState_Circle2x2LeftDown() {
    move_on_circle3(16.0f, 651.899169921875f);
}

void dLineMng_c::initializeState_Circle2x2RightDown() {}
void dLineMng_c::finalizeState_Circle2x2RightDown() {}
void dLineMng_c::executeState_Circle2x2RightDown() {
    move_on_circle4(16.0f, 651.899169921875f);
}

void dLineMng_c::initializeState_Circle4x4Rightup() {}
void dLineMng_c::finalizeState_Circle4x4Rightup() {}
void dLineMng_c::executeState_Circle4x4Rightup() {
    move_on_circle1(32.0f, 325.9495849609375f);
}

void dLineMng_c::initializeState_Circle4x4LeftUp() {}
void dLineMng_c::finalizeState_Circle4x4LeftUp() {}
void dLineMng_c::executeState_Circle4x4LeftUp() {
    move_on_circle2(32.0f, 325.9495849609375f);
}

void dLineMng_c::initializeState_Circle4x4LeftDown() {}
void dLineMng_c::finalizeState_Circle4x4LeftDown() {}
void dLineMng_c::executeState_Circle4x4LeftDown() {
    move_on_circle3(32.0f, 325.9495849609375f);
}

void dLineMng_c::initializeState_Circle4x4RightDown() {}
void dLineMng_c::finalizeState_Circle4x4RightDown() {}
void dLineMng_c::executeState_Circle4x4RightDown() {
    move_on_circle4(32.0f, 325.9495849609375f);
}
