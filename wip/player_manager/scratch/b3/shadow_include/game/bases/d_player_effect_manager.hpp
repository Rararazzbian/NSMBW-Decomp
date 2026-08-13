#pragma once

#include <game/mLib/m_vec.hpp>

class dPyEffectMng_c {
public:
    void fn_800d2de0(float, int, mVec3_c &, u8); ///< @unofficial
    void update(); ///< SHADOW-ONLY addition, see BATCH3.md.

    static dPyEffectMng_c *mspInstance;
};
