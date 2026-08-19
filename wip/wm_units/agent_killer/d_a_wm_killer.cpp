#include <game/bases/d_a_wm_killer.hpp>

ACTOR_PROFILE(WM_KILLER, daWmKiller_c, 0);

daWmKiller_c::daWmKiller_c() : m_208(false) {}
daWmKiller_c::~daWmKiller_c() {}

int daWmKiller_c::execute() {
    return SUCCEEDED;
}
