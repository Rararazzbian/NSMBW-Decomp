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
