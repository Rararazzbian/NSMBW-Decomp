
#include <game/bases/d_wm_lib.hpp>
#include "d_a_wm_grid.hpp"

// [This is required to ensure correct .rodata pool ordering: the pool must
//  open with a 0.0f that no placed function references.]
// [It will be deadstripped by the linker later.]
DECL_WEAK
void DUMMY_ORDERING() {
    static const float UNUSED[] = { 0.0f };
}

ACTOR_PROFILE(WM_GRID, daWmGrid_c, 0);

daWmGrid_c::daWmGrid_c() {}
daWmGrid_c::~daWmGrid_c() {}

int daWmGrid_c::create() {
    return SUCCEEDED;
}

int daWmGrid_c::execute() {
    return SUCCEEDED;
}

int daWmGrid_c::draw() {
    return SUCCEEDED;
}

int daWmGrid_c::doDelete() {
    return SUCCEEDED;
}

int daWmGrid_c::GetActorType() {
    return 0;
}
