#pragma once
#include <game/mLib/m_vec.hpp>
#include "d_py_effect_stub.hpp"

class dPyEffectMng_c {
public:
    virtual ~dPyEffectMng_c() {}

    void fn_800d2de0(float, int, mVec3_c &, u8);

    // sizeof(dPyEffectMng_c) == 0xC5C (3164 bytes)
    // vptr (4) + 10 ? dPyEffect_c (10 ? 0x13C = 0xC58) = 0xC5C
    dPyEffect_c mEffects[10];

    static dPyEffectMng_c *mspInstance;
};

static_assert(sizeof(dPyEffectMng_c) == 0xC5C, "dPyEffectMng_c size wrong");
