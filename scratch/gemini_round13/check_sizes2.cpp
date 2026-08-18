#include <game/bases/d_wm_lib.hpp>
#include "d_a_wm_grid.hpp"
#include "d_a_wm_tower.hpp"

size_t sz_grid = sizeof(daWmGrid_c);
size_t sz_tower = sizeof(daWmTower_c);
size_t ofs_grid_alloc = offsetof(daWmGrid_c, mAllocator);
size_t ofs_grid_mdl = offsetof(daWmGrid_c, mModel);
size_t ofs_tower_idx = offsetof(daWmTower_c, mResNodeIdx);
size_t ofs_tower_alloc = offsetof(daWmTower_c, mAllocator);
size_t ofs_tower_mdl = offsetof(daWmTower_c, mModel);
