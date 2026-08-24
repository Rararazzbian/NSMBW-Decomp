#include <game/bases/d_lift_allhit_draw2.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_bg.hpp>

// Anonymous .sdata2 pool literals owned by this TU's address block, pinned by
// address so the TU emits no pool of its own.
extern const float l_poolZero_8042CA70;
extern const float l_poolOne_8042CA90;

void fn_800BFB60(void *, float *, u8, u8, u8, s16);

// .bss:0x803590F0, size 0x10 -- three floats runtime-initialised to the pool
// literal at .sdata2:0x8042CA90 by __sinit_d_lift_allhit_draw2_cpp; the fourth
// word is never touched.
struct LiftScaleInit {
    float a, b, c, d;
    LiftScaleInit(float v) : a(v), b(v), c(v) {}
};
LiftScaleInit s_liftScale(l_poolOne_8042CA90);

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
            v.x += bg;
        } else {
            v.x -= bg;
        }
        fn_800BFB60(mUnk22ac, &v.x, a, b, c, ang);
    }
}
