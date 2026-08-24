#include <game/bases/d_a_player_base.hpp>
#include <game/framework/f_manager.hpp>
#include <game/bases/d_funsui_act.hpp>

static inline u8 &field_38c_ref(daPlBase_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x38c);
}

void dFunsuiAct_c::posMove() {
    static const float cs_rev_speed[2] = { 3.0f, -3.0f };
    daPlBase_c *p = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);
    if (p == NULL) {
        return;
    }

    switch (field_38c_ref(p)) {
    case 1:
    case 2: {
        daPlBase_c *pl = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);

        if (mTimer > 0) {
            mTimer--;
            mTargetY += cs_rev_speed[mRevIndex];
            mSpeed = 0.0f;
        } else {
            mTargetY = pl->mPos.x;
        }
        mCurrentY += mSpeed;
        pl->updateFunsuiPos(mTargetY, mCurrentY);
        break;
    }
    case 3: {
        mVec2_c probe(p->mPos.x, mCurrentY + mSpeed);
        mTargetY = probe.x;
        mCurrentY = probe.y;
        p->mPos.x = probe.x;
        p->mPos.y = probe.y;
        dBaseActor_c *a = p;
        mVec3_c v(a->getCenterPos().x, a->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
        break;
    }
    }
}
