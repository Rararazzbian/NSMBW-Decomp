#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <game/bases/d_game_com.hpp>

static const float l_shellatk_speed[2] = { 4.0f, -4.0f };

void dEnTorideKokoopa_c::initializeState_ShellAtk() {
    float left = dGameCom::getDispCenterX() + mUnk840;
    float right = dGameCom::getDispCenterX() + mUnk844;
    float center = 0.5f * (left + right);
    mDirection = (mPos.x >= center) ? 0 : 1;
    mActorProperties &= ~0x200;
    mUnkAC8 = 5;
    mAccelF = 0.3f;
    mSpeed.x = l_shellatk_speed[mDirection];
    if (mPos.x - 32.0f < right) {
        mSpeedMax.x = mSpeed.x;
        mUnkAC4 = 32.0f + right;
        mUnkAC8 = 6;
    } else if (mPos.x + 32.0f > left) {
        mSpeedMax.x = mSpeed.x;
        mUnkAC4 = left - 32.0f;
        mUnkAC8 = 6;
    } else {
        mUnkAC4 = mPos.x;
        mSpeedMax.x = l_shellatk_speed[mDirection ^ 1];
    }
}
