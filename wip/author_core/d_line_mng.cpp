#include <game/bases/d_line_mng.hpp>
#include <cmath>
#include <nw4r/math/math_triangular.h>

// LOCAL TEST ONLY -- including the real <game/bases/d_bc.hpp> pulls in
// d_bg_ctr.hpp/d_actor.hpp and fails to compile standalone (undefined
// dBg_ctr_c, illegal 'virtual' outside a class -- a pre-existing issue in
// that header chain when compiled in isolation, not something introduced
// here). getUnitType/getUnitKind are declared `static u32 (float,float,u8)`
// in the real include/game/bases/d_bc.hpp:217-218; this minimal forward
// declaration reproduces the exact mangled signature
// (getUnitType__5dBc_cFffUc / getUnitKind__5dBc_cFffUc) without dragging in
// the rest of the actor hierarchy. ASSUMED HELPER SIGNATURE -- flag for
// merge review.
class dBc_c {
public:
    static u32 getUnitType(float x, float y, u8 layer);
    static u32 getUnitKind(float x, float y, u8 layer);
};

// LOCAL TEST ONLY -- smc_UNIT_SIZE_X is not in the shared header yet; see
// RESULT.md for the proposed header addition. Value 16.0f read directly out
// of original/wiimj2d.dol at .sdata2:0x8042CB18 (raw bytes 41 80 00 00).
// Deliberately NOT defined/initialised in this TU -- see RESULT.md: defining
// it here let MWCC constant-fold the division into a multiply-by-reciprocal
// (proven: draft emitted fmuls with zero fdivs where target has two fdivs).
// Its real definition must live in a DIFFERENT TU not yet decompiled.

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

void dLineMng_c::SetBaseSpeed(f32 speed)
{
    if (mReverse) speed = -speed;
    mBaseSpeed = speed;
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

void dLineMng_c::init_term_ck_pos()
{
    mVec2_c *p = mDirVec;
    do {
        p->x = 0.0f;
        p->y = 0.0f;
        p++;
    } while (p != mDirVec + 3);

    static mVec2_POD_c d_dir[4];
    static u8 s_init = 0;
    if (!s_init) {
        d_dir[0].set(-0.1f, 0.1f);
        d_dir[1].set(0.1f, 0.1f);
        d_dir[2].set(-0.1f, -0.1f);
        d_dir[3].set(0.1f, -0.1f);
        s_init = 1;
    }

    const mVec2_POD_c *q = d_dir;
    do {
        p->x = q->x;
        p->y = q->y;
        q++;
        p++;
    } while (p != mDirVec + 7);
}

bool dLineMng_c::check_term()
{
    mVec2_c *end = mDirVec + 7;
    mVec2_c *p = mDirVec;
    do {
        if (getLineUnitNo((f32)(int)((mPos.x + p->x) / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X,
                           (f32)(int)((mPos.y + p->y) / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X) == 0x22) {
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

void dLineMng_c::start_line_move()
{
    double t = std::fmod(mPos.x, 16.0);
    int triggerFallDown = 0;
    mUnk6a = triggerFallDown;
    if (t > -0.1 && t < 0.1) {
        mVec2_c snap;
        snap.x = (f32)(int)(mPos.x / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X - 16.0f;
        snap.y = (f32)(int)(mPos.y / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X;
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

void dLineMng_c::initializeState_Idle() {}
void dLineMng_c::finalizeState_Idle() {}
void dLineMng_c::executeState_Idle() {}

void dLineMng_c::initializeState_FallDown() {}
void dLineMng_c::finalizeState_FallDown() {}
void dLineMng_c::executeState_FallDown() {}

void dLineMng_c::initializeState_Left45() {}
void dLineMng_c::finalizeState_Left45() {}
void dLineMng_c::executeState_Left45() {}

void dLineMng_c::initializeState_Right45() {}
void dLineMng_c::finalizeState_Right45() {}
void dLineMng_c::executeState_Right45() {}

void dLineMng_c::initializeState_Side() {}
void dLineMng_c::finalizeState_Side() {}
void dLineMng_c::executeState_Side() {}

void dLineMng_c::initializeState_Height() {}
void dLineMng_c::finalizeState_Height() {}
void dLineMng_c::executeState_Height() {}

void dLineMng_c::initializeState_CornerHeightLine() {}
void dLineMng_c::finalizeState_CornerHeightLine() {}
void dLineMng_c::executeState_CornerHeightLine() {}

void dLineMng_c::initializeState_CornerSideLine() {}
void dLineMng_c::finalizeState_CornerSideLine() {}
void dLineMng_c::executeState_CornerSideLine() {}

void dLineMng_c::initializeState_Left30Left() {}
void dLineMng_c::finalizeState_Left30Left() {}
void dLineMng_c::executeState_Left30Left() {}

void dLineMng_c::initializeState_Left30Right() {}
void dLineMng_c::finalizeState_Left30Right() {}
void dLineMng_c::executeState_Left30Right() {}

void dLineMng_c::initializeState_Right30Left() {}
void dLineMng_c::finalizeState_Right30Left() {}
void dLineMng_c::executeState_Right30Left() {}

void dLineMng_c::initializeState_Right30Right() {}
void dLineMng_c::finalizeState_Right30Right() {}
void dLineMng_c::executeState_Right30Right() {}

void dLineMng_c::initializeState_Left60Up() {}
void dLineMng_c::finalizeState_Left60Up() {}
void dLineMng_c::executeState_Left60Up() {}

void dLineMng_c::initializeState_Left60Down() {}
void dLineMng_c::finalizeState_Left60Down() {}
void dLineMng_c::executeState_Left60Down() {}

void dLineMng_c::initializeState_Right60Down() {}
void dLineMng_c::finalizeState_Right60Down() {}
void dLineMng_c::executeState_Right60Down() {}

void dLineMng_c::initializeState_Right60Up() {}
void dLineMng_c::finalizeState_Right60Up() {}
void dLineMng_c::executeState_Right60Up() {}

void dLineMng_c::initializeState_Circle() {}
void dLineMng_c::finalizeState_Circle() {}
void dLineMng_c::executeState_Circle() {}

void dLineMng_c::initializeState_Circle2x2Leftup() {}
void dLineMng_c::finalizeState_Circle2x2Leftup() {}
void dLineMng_c::executeState_Circle2x2Leftup() {}

void dLineMng_c::initializeState_Circle2x2Rightup() {}
void dLineMng_c::finalizeState_Circle2x2Rightup() {}
void dLineMng_c::executeState_Circle2x2Rightup() {}

void dLineMng_c::initializeState_Circle2x2LeftDown() {}
void dLineMng_c::finalizeState_Circle2x2LeftDown() {}
void dLineMng_c::executeState_Circle2x2LeftDown() {}

void dLineMng_c::initializeState_Circle2x2RightDown() {}
void dLineMng_c::finalizeState_Circle2x2RightDown() {}
void dLineMng_c::executeState_Circle2x2RightDown() {}

void dLineMng_c::initializeState_Circle4x4Rightup() {}
void dLineMng_c::finalizeState_Circle4x4Rightup() {}
void dLineMng_c::executeState_Circle4x4Rightup() {}

void dLineMng_c::initializeState_Circle4x4LeftUp() {}
void dLineMng_c::finalizeState_Circle4x4LeftUp() {}
void dLineMng_c::executeState_Circle4x4LeftUp() {}

void dLineMng_c::initializeState_Circle4x4LeftDown() {}
void dLineMng_c::finalizeState_Circle4x4LeftDown() {}
void dLineMng_c::executeState_Circle4x4LeftDown() {}

void dLineMng_c::initializeState_Circle4x4RightDown() {}
void dLineMng_c::finalizeState_Circle4x4RightDown() {}
void dLineMng_c::executeState_Circle4x4RightDown() {}

// --- Unnamed file-scope helpers (internal linkage; dtk shows fn_<addr>
// placeholders because they carry no mangled class-scope name). Placement
// here is for standalone testing only -- see RESULT.md for their required
// position in the merged file (address-interleaved with the named methods,
// per the project's "definition order is part of the object" rule).

/// @unofficial fn_800C15B0 (0x1C bytes, 7 words). Shape: arr[idx] = *src for
/// an 8-byte-stride array, confirmed in MAPPING.md. Sits between acm_angle()
/// and start_line_move() in the target's .text address order.
static void setArrElem_800C15B0(mVec2_c *arr, const mVec2_c *src, int idx)
{
    arr[idx] = *src;
}

// LOCAL TEST ONLY -- setArrElem_800C15B0 has no caller yet in this draft
// (its real caller is presumably fn_800C31C0 or another not-yet-authored
// function), so a plain `static` definition is dead-stripped by MWCC and
// never emitted for diffing. This dummy forces emission for the standalone
// diff check only; DELETE at merge once a real caller exists.
void DUMMY_FORCE_EMIT_800C15B0(mVec2_c *arr, const mVec2_c *src, int idx)
{
    setArrElem_800C15B0(arr, src, idx);
}

/// @unofficial fn_800C3B20 (0x3C bytes, 15 words). Clamps mPos.x into
/// [mUnitBasePos.x, mUnitBasePos.x+16.0) against the near edge (0.1f margin
/// on the upper side). Placeholder name -- genuinely anonymous in the binary.
void clampPosX_800C3B20(dLineMng_c *self)
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

/// @unofficial fn_800C3B60 (0x3C bytes, 15 words). Mirrors
/// clampPosX_800C3B20 for the Y axis.
void clampPosY_800C3B60(dLineMng_c *self)
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
