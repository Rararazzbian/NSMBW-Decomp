#pragma once

#include <game/bases/d_effect.hpp>
#include <game/mLib/m_vec.hpp>

/// @brief One player-linked effect slot. Embeds a dEf::followEffect_c.
/// @details sizeof 0x13C, fixed by dPyEffectMng_c's array stride.
/// __vt__11dPyEffect_c is 0xC, one virtual slot -- only the destructor is virtual.
/// @unofficial
class dPyEffect_c {
public:
    virtual ~dPyEffect_c();

    void update();

    dEf::followEffect_c mEffect;
    mVec3_c mPosition;
    mVec3_c mScale;
    u8 mLayer;
    u8 mPad131[3];
    int mEffectId;
    int mActive;
};

static_assert(sizeof(EGG::Effect) == 0x114, "EGG::Effect size wrong");
static_assert(sizeof(mEf::effect_c) == 0x114, "mEf::effect_c size wrong");
static_assert(sizeof(dEf::followEffect_c) == 0x114, "dEf::followEffect_c size wrong");
static_assert(sizeof(dPyEffect_c) == 0x13C, "dPyEffect_c size wrong");
