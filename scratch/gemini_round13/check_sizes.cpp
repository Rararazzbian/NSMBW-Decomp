#include <game/bases/d_wm_lib.hpp>
#include "d_a_wm_grid.hpp"
#include "d_a_wm_tower.hpp"

#define STATIC_ASSERT(cond) typedef char __static_assert_t[(cond) ? 1 : -1]

STATIC_ASSERT(sizeof(daWmGrid_c) == 0x170);
STATIC_ASSERT(offsetof(daWmGrid_c, mAllocator) == 0x138);
STATIC_ASSERT(offsetof(daWmGrid_c, mModel) == 0x154);

STATIC_ASSERT(sizeof(daWmTower_c) == 0x1C0);
STATIC_ASSERT(offsetof(daWmTower_c, mResNodeIdx) == 0x184);
STATIC_ASSERT(offsetof(daWmTower_c, mAllocator) == 0x188);
STATIC_ASSERT(offsetof(daWmTower_c, mModel) == 0x1A4);
