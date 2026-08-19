#include <types.h>
#include <game/bases/d_a_wm_dance_pakkun.hpp>
#include <stddef.h>

STATIC_ASSERT(sizeof(daWmDancePakkun_c) == 0x2e0);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, m_184) == 0x184);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, m_188) == 0x188);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, mAllocator) == 0x18c);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, mModel) == 0x1ac);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, mModel2) == 0x1ec);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, mChrAnim) == 0x22c);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, mBlend) == 0x264);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, mTexSrt) == 0x28c);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, m_2b8) == 0x2b8);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, m_2bc) == 0x2bc);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, pad_2c0) == 0x2c0);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, m_2d8) == 0x2d8);
STATIC_ASSERT(offsetof(daWmDancePakkun_c, mBgmSync) == 0x2dc);

template<int N> struct Probe;
class Probe_c : public daWmDancePakkun_c {
public:
    Probe<offsetof(daWmDancePakkun_c, mModel)> *a1;
    Probe<offsetof(daWmDancePakkun_c, mModel2)> *a2;
    Probe<offsetof(daWmDancePakkun_c, mChrAnim)> *a3;
    Probe<offsetof(daWmDancePakkun_c, mBlend)> *a4;
    Probe<offsetof(daWmDancePakkun_c, mTexSrt)> *a5;
    Probe<offsetof(daWmDancePakkun_c, m_2b8)> *a6;
    Probe<sizeof(daWmDancePakkun_c)> *a7;
};
void useP(Probe_c *x) {
    *x->a1 = *x->a1;
    *x->a2 = *x->a2;
    *x->a3 = *x->a3;
    *x->a4 = *x->a4;
    *x->a5 = *x->a5;
    *x->a6 = *x->a6;
    *x->a7 = *x->a7;
}

int dummy2() { return 0; }
