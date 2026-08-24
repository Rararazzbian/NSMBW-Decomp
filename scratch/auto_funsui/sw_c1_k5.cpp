#include <game/bases/d_a_player_base.hpp>
#include <game/framework/f_manager.hpp>
#include <game/bases/d_funsui_act.hpp>

extern const float l_zero_8042C848;
extern const float l_5500_8042C84C;
extern const char l_str_803158C8[16];
extern const float l_revSpeed_8042C840[2];

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
        daPlBase_c *pl = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);

        if (mTimer > 0) {
            mTimer--;
            mSpeed = l_zero_8042C848;
            mTargetY += l_revSpeed_8042C840[mRevIndex];
        } else {
            mTargetY = pl->mPos.x;
        }
        mCurrentY += mSpeed;
        pl->updateFunsuiPos(mTargetY, mCurrentY);
        break;
    case 3: {
        float nc = mCurrentY + mSpeed;
        mTargetY = p->mPos.x;
        mCurrentY = nc;
        p->mPos.x = mTargetY;
        p->mPos.y = nc;

        mVec3_c v;
        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();
        mVec2_c d;
        d.x = mTargetY;
        d.y = mCurrentY;

        v.x = c1.x;
        v.y = c2.y;
        v.z = l_5500_8042C84C;
        mEffect.createEffect(l_str_803158C8, 0, &v, NULL, NULL);
        break;
    }
    }
    }
}
