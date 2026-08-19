#include <game/bases/d_a_wm_killer.hpp>

ACTOR_PROFILE(WM_KILLER, daWmKiller_c, 0);

daWmKiller_c::daWmKiller_c() : m_208(false) {}
daWmKiller_c::~daWmKiller_c() {}

// NOT YET AUTHORED (0x6c bytes). Vtable slot 2, confirmed via check_vtable.py. Distinct
// placeholder body (not `return SUCCEEDED;`) so this doesn't collide with #doDelete's real,
// confirmed trivial body in verify_anon's content-pairing.
int daWmKiller_c::create() {
    m_208 = true;
    return SUCCEEDED;
}

// NOT YET AUTHORED (0x124 bytes). Vtable slot 8, confirmed via check_vtable.py.
int daWmKiller_c::execute() {
    m_208 = false;
    return SUCCEEDED;
}

int daWmKiller_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmKiller_c::doDelete() {
    return SUCCEEDED;
}

// NOT YET AUTHORED (0x88 bytes). Vtable slot 24, confirmed via check_vtable.py.
void daWmKiller_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    (void) cutsceneCommandId;
    (void) isFirstFrame;
}
