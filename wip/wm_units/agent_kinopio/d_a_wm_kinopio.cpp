#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_a_wm_kinopio.hpp>

ACTOR_PROFILE(WM_KINOPIO, daWmKinopio_c, 0);

daWmKinopio_c::daWmKinopio_c() {}

daWmKinopio_c::~daWmKinopio_c() {
    if (mpMdlMng) {
        delete mpMdlMng;
    }
}

int daWmKinopio_c::draw() {
    mpMdlMng->draw();
    DrawShadow(true);
    return SUCCEEDED;
}

int daWmKinopio_c::doDelete() {
    return 1;
}

void daWmKinopio_c::resetStep() {
    m_190 = 0;
}

void daWmKinopio_c::unusedStub() {
}
