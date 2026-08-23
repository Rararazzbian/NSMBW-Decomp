#include "d_panel_obj_list.hpp"
#include <cstddef>

#define STATIC_ASSERT(cond) typedef char static_assert_failed[(cond) ? 1 : -1]

STATIC_ASSERT(sizeof(dPanelObjList_c) == 0x20);
STATIC_ASSERT(offsetof(dPanelObjList_c, mpPrev) == 0x00);
STATIC_ASSERT(offsetof(dPanelObjList_c, mpNext) == 0x04);
STATIC_ASSERT(offsetof(dPanelObjList_c, mValue) == 0x08);
STATIC_ASSERT(offsetof(dPanelObjList_c, mType) == 0x0A);
STATIC_ASSERT(offsetof(dPanelObjList_c, mChange) == 0x0B);
STATIC_ASSERT(offsetof(dPanelObjList_c, mPosX) == 0x0C);
STATIC_ASSERT(offsetof(dPanelObjList_c, mPosY) == 0x10);
STATIC_ASSERT(offsetof(dPanelObjList_c, mPosZ) == 0x14);
STATIC_ASSERT(offsetof(dPanelObjList_c, mScale) == 0x18);
STATIC_ASSERT(offsetof(dPanelObjList_c, mAngle) == 0x1C);
STATIC_ASSERT(offsetof(dPanelObjList_c, mParts) == 0x1E);