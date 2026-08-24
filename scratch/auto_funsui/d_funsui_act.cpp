#include <game/bases/d_funsui_act.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/framework/f_manager.hpp>

/// @unofficial 0x38c is not named in the frozen headers; see the landed
/// d_a_player_demo_manager.cpp for the same raw-cast pattern.
static inline u8 &field_38c_ref(daPlBase_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x38c);
}

void dFunsuiAct_c::posMove() {
    daPlBase_c *p = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);
    if (p == NULL) {
        return;
    }

    switch (field_38c_ref(p)) {
    case 1:
    case 2: {
        static const float cs_rev_speed[2] = { 3.0f, -3.0f };

        daPlBase_c *player = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);
        if (mTimer > 0) {
            mTimer--;
            mSpeed = 0.0f;
            mTargetY += cs_rev_speed[mRevIndex];
        } else {
            mTargetY = player->mPos.y;
        }
        mCurrentY += mSpeed;
        player->updateFunsuiPos(mTargetY, mCurrentY);
        break;
    }
    case 3: {
        float py = p->mPos.y;
        mTargetY = py;
        mCurrentY += mSpeed;
        p->mPos.y = py;
        p->mPos.z = mCurrentY;

        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();

        mVec3_c v;
        v.x = c1.x;
        v.y = c2.y;
        v.z = 5500.0f;

        mObj.exec(p, 0, v, 0, 0);
        break;
    }
    }
}
