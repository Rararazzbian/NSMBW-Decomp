#include <game/bases/d_player_effect_manager.hpp>

dPyEffectMng_c *dPyEffectMng_c::mspInstance;

dPyEffectMng_c::dPyEffectMng_c() {
    mspInstance = this;
}

dPyEffectMng_c::~dPyEffectMng_c() {}

bool dPyEffectMng_c::fn_800d2de0(f32 f, int i, mVec3_c &pos, u8 layer) {
    for (int n = 0; n < 10; n++) {
        if (fn_800D2BB0(&mEffects[n], f, i, pos, layer)) {
            return true;
        }
    }
    return false;
}

void dPyEffectMng_c::update() {
    for (int n = 0; n < 10; n++) {
        mEffects[n].update();
    }
}
