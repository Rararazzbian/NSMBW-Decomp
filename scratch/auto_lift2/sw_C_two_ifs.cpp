#include <game/bases/d_lift_allhit_draw2.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_bg.hpp>

extern const float l_poolZero_8042CA70;
extern const float l_poolOne_8042CA90;

void fn_800BFB60(void *, float *, u8, u8, u8, s16);

void dLiftAllhitDraw2_c::draw() {
    u8 a = mColA;
    u8 b = mColB;
    u8 c = mColC;
    s16 ang = mAng;

    fn_800BFB60(mUnk1c, &mMinY, a, b, c, ang);

    if (dScStage_c::m_loopType == 1) {
        f32 z = mMaxY;
        f32 mid = mMidY;
        f32 y = mMinY;
        mVec3_c v(y, mid, z);
        f32 bg = dBg_c::m_bg_p->mLoopOffset;
        if (y < l_poolZero_8042CA70) {
            v.x = y + bg;
        }
        if (y >= l_poolZero_8042CA70) {
            v.x = y - bg;
        }

        fn_800BFB60(mUnk22ac, &v.x, a, b, c, ang);
    }
}
