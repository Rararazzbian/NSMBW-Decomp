#include <game/bases/d_line_mng.hpp>

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
