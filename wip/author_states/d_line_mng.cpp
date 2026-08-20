#include <game/bases/d_line_mng.hpp>

// Unnamed file-scope helpers (see wip/agent_line_mng/MAPPING.md's "Unnamed
// file-scope functions" table). No mangled class-scope name exists for these
// in the symbol map, so they are almost certainly static free functions in
// this TU, not dLineMng_c members -- ASSUMED signatures below, not proven.
// Author's note: these bodies are owned by the geom/mov agents; these are
// forward declarations only, so this file compiles standalone. Named here by
// address per the project's un-landed-callee convention.
static void fn_800C3B20(dLineMng_c *); ///< @unofficial ASSUMED sig: (this). Reads mUnitBasePos (0x50/0x54).
static void fn_800C3B60(dLineMng_c *); ///< @unofficial ASSUMED sig: (this). Reads mUnitBasePos.y (0x54).

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

void dLineMng_c::initializeState_Idle() {}
void dLineMng_c::finalizeState_Idle() {}
void dLineMng_c::executeState_Idle() {}

void dLineMng_c::initializeState_FallDown() {}
void dLineMng_c::finalizeState_FallDown() {}
void dLineMng_c::executeState_FallDown() {}

void dLineMng_c::initializeState_Left45() {
    fn_800C3B20(this);
    mAngle = 0xE000;
    mPos.y = (mUnitBasePos.y - 16.0f) + (mPos.x - mUnitBasePos.x);
}
void dLineMng_c::finalizeState_Left45() {}
void dLineMng_c::executeState_Left45() {
    mVec2_c old = mPos;
    f32 dv = mBaseSpeed * 0.70703f;
    mSpeed.x = dv;
    mSpeed.y = dv;
    mPos.x += dv;
    mPos.y = (mUnitBasePos.y - 16.0f) + (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        f32 dv2 = mBaseSpeed * 0.70703f;
        mSpeed.x = dv2;
        mSpeed.y = dv2;
    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mov_frm_rightupper(mUnitBasePos, false);
    }
}

void dLineMng_c::initializeState_Right45() {
    fn_800C3B20(this);
    mAngle = 0xA000;
    mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x);
}
void dLineMng_c::finalizeState_Right45() {}
void dLineMng_c::executeState_Right45() {
    mVec2_c old = mPos;
    f32 dv = mBaseSpeed * 0.70703f;
    mSpeed.x = dv;
    mSpeed.y = -dv;
    mPos.x += dv;
    mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        f32 dv2 = mBaseSpeed * 0.70703f;
        mSpeed.x = dv2;
        mSpeed.y = -dv2;
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
        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
        if (lineUnitNo == 8) {
            mUnitBasePos.x = mUnitBasePos.x + 16.0f;
            mStateMgr.changeState(StateID_Left30Right);
        } else if (lineUnitNo == 0xA) {
            mUnitBasePos.x = mUnitBasePos.x + 16.0f;
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
    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 8.0f) + 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = 0.5f * mSpeed.x;
    } else if (mPos.x < mUnitBasePos.x) {
        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x - 16.0f, mUnitBasePos.y);
        if (lineUnitNo == 9) {
            mUnitBasePos.x = mUnitBasePos.x - 16.0f;
            mStateMgr.changeState(StateID_Left30Left);
        } else if (lineUnitNo == 0xB) {
            mUnitBasePos.x = mUnitBasePos.x - 16.0f;
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
        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
        if (lineUnitNo == 8) {
            mVec2_c newBase(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
            mUnitBasePos = newBase;
            mStateMgr.changeState(StateID_Left30Right);
        } else if (lineUnitNo == 0xA) {
            mVec2_c newBase(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
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
    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = -(0.5f * mSpeed.x);
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 8.0f) - 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = -(0.5f * mSpeed.x);
    } else if (mPos.x < mUnitBasePos.x) {
        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x - 16.0f, mUnitBasePos.y);
        if (lineUnitNo == 9) {
            mUnitBasePos.x = mUnitBasePos.x - 16.0f;
            mStateMgr.changeState(StateID_Left30Left);
        } else if (lineUnitNo == 0xB) {
            mVec2_c newBase(mUnitBasePos.x - 16.0f, mUnitBasePos.y);
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
    mSpeed.y = mBaseSpeed * 0.8910065f;
    mSpeed.x = 0.5f * mSpeed.y;
    mPos.y += mSpeed.y;
    mPos.x = (mUnitBasePos.x + 16.0f) - 0.5 * (mUnitBasePos.y - mPos.y);
    if (check_term()) {
        mPos = old;
        mSpeed.y = mBaseSpeed * 0.8910065f;
        mSpeed.x = 0.5f * mSpeed.y;
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x, mUnitBasePos.y - 16.0f);
        if (lineUnitNo == 0xD) {
            mUnitBasePos.y = mUnitBasePos.y - 16.0f;
            mStateMgr.changeState(StateID_Left60Down);
        } else if (lineUnitNo == 0xE) {
            mUnitBasePos.y = mUnitBasePos.y - 16.0f;
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
    mSpeed.y = mBaseSpeed * 0.8910065f;
    mSpeed.x = 0.5f * mSpeed.y;
    mPos.y += mSpeed.y;
    f32 t = mUnitBasePos.x + 8.0;
    mPos.x = t - 0.5 * (mUnitBasePos.y - mPos.y);
    if (check_term()) {
        mPos = old;
        mSpeed.y = mBaseSpeed * 0.8910065f;
        mSpeed.x = 0.5f * mSpeed.y;
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.y >= mUnitBasePos.y) {
        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x, mUnitBasePos.y + 16.0f);
        if (lineUnitNo == 0xC) {
            mUnitBasePos.y = mUnitBasePos.y + 16.0f;
            mStateMgr.changeState(StateID_Left60Up);
        } else if (lineUnitNo == 0xF) {
            mUnitBasePos.y = mUnitBasePos.y + 16.0f;
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
    mSpeed.y = mBaseSpeed * -0.8910065f;
    mSpeed.x = -(0.5f * mSpeed.y);
    mPos.y += mSpeed.y;
    f32 t = mUnitBasePos.x + 8.0;
    mPos.x = t + 0.5 * (mUnitBasePos.y - mPos.y);
    if (check_term()) {
        mPos = old;
        mSpeed.y = mBaseSpeed * -0.8910065f;
        mSpeed.x = -(0.5f * mSpeed.y);
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        mov_frm_rightlower(mUnitBasePos, false);
    } else if (mPos.y >= mUnitBasePos.y) {
        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x, mUnitBasePos.y + 16.0f);
        if (lineUnitNo == 0xC) {
            mUnitBasePos.y = mUnitBasePos.y + 16.0f;
            mStateMgr.changeState(StateID_Left60Up);
        } else if (lineUnitNo == 0xF) {
            mUnitBasePos.y = mUnitBasePos.y + 16.0f;
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
    mSpeed.y = mBaseSpeed * -0.8910065f;
    mSpeed.x = -(0.5f * mSpeed.y);
    mPos.y += mSpeed.y;
    mPos.x = mUnitBasePos.x + 0.5 * (mUnitBasePos.y - mPos.y);
    if (check_term()) {
        mPos = old;
        mSpeed.y = mBaseSpeed * -0.8910065f;
        mSpeed.x = -(0.5f * mSpeed.y);
    } else if (mPos.y < mUnitBasePos.y - 16.0f) {
        u32 lineUnitNo = getLineUnitNo(mUnitBasePos.x, mUnitBasePos.y - 16.0f);
        if (lineUnitNo == 0xD) {
            mUnitBasePos.y = mUnitBasePos.y - 16.0f;
            mStateMgr.changeState(StateID_Left60Down);
        } else if (lineUnitNo == 0xE) {
            mUnitBasePos.y = mUnitBasePos.y - 16.0f;
            mStateMgr.changeState(StateID_Right60Down);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    } else if (mPos.y >= mUnitBasePos.y) {
        mov_frm_leftupper(mUnitBasePos, false);
    }
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

void dLineMng_c::finalizeState_Circle2x2Leftup() {}
void dLineMng_c::executeState_Circle2x2Leftup() {
    move_on_circle2(16.0f, 651.899169921875f);
}

void dLineMng_c::finalizeState_Circle2x2Rightup() {}
void dLineMng_c::executeState_Circle2x2Rightup() {
    move_on_circle1(16.0f, 651.899169921875f);
}

void dLineMng_c::finalizeState_Circle2x2LeftDown() {}
void dLineMng_c::executeState_Circle2x2LeftDown() {
    move_on_circle3(16.0f, 651.899169921875f);
}

void dLineMng_c::finalizeState_Circle2x2RightDown() {}
void dLineMng_c::executeState_Circle2x2RightDown() {
    move_on_circle4(16.0f, 651.899169921875f);
}

void dLineMng_c::finalizeState_Circle4x4Rightup() {}
void dLineMng_c::executeState_Circle4x4Rightup() {
    move_on_circle1(32.0f, 325.9495849609375f);
}

void dLineMng_c::finalizeState_Circle4x4LeftUp() {}
void dLineMng_c::executeState_Circle4x4LeftUp() {
    move_on_circle2(32.0f, 325.9495849609375f);
}

void dLineMng_c::finalizeState_Circle4x4LeftDown() {}
void dLineMng_c::executeState_Circle4x4LeftDown() {
    move_on_circle3(32.0f, 325.9495849609375f);
}

void dLineMng_c::finalizeState_Circle4x4RightDown() {}
void dLineMng_c::executeState_Circle4x4RightDown() {
    move_on_circle4(32.0f, 325.9495849609375f);
}
