#pragma once

#include <game/mLib/m_vec.hpp>
#include <game/bases/d_effect.hpp>

/// @brief One player-linked effect slot.
/// @details sizeof 0x13C, fixed by dPyEffectMng_c's array stride.
/// `__vt__11dPyEffect_c` is 0xC -- one virtual slot -- so the destructor is the
/// only virtual function and `update()` is NOT virtual, despite existing as
/// `update__11dPyEffect_cFv`.
///
/// The constructor at 0x800D2AE0 stores dPyEffect_c's vtable at `this+0`, then
/// constructs an EGG::Effect at `this+4` and overwrites that embedded object's
/// vptr with `__vt__Q23dEf14followEffect_c` -- so the whole unexplained
/// 0x004..0x118 region is ONE embedded dEf::followEffect_c, whose sizeof is
/// 0x114 (verified by probe). That lands the trailing fields exactly.
/// @unofficial
class dPyEffect_c {
public:
    /// @note Declared, NOT defined inline. dPyEffectMng_c embeds ten of these
    /// by value, so an implicit constructor would make MWCC synthesise one
    /// here -- and it would drag in weak copies of followEffect_c's,
    /// mEf::effect_c's and mVec3_c's constructors and destructors, none of
    /// which the original emits in this TU. The real one is at 0x800D2AE0.
    dPyEffect_c();
    virtual ~dPyEffect_c();

    void update();

    dEf::followEffect_c mEffect; ///< [0x004] 0x114 bytes.
    mVec3_c mPosition;           ///< [0x118]
    mVec3_c mScale;              ///< [0x124]
    u8 mLayer;                   ///< [0x130]
    u8 pad131[3];                ///< [0x131]
    int mEffectId;               ///< [0x134]
    int mActive;                 ///< [0x138]
};

STATIC_ASSERT(sizeof(dPyEffect_c) == 0x13C);

/// @brief Owns the ten player-linked effect slots.
/// @details [.bss instance at 0x80355354, sizeof 0xC5C]. `__vt__14dPyEffectMng_c`
/// is 0xC -- one virtual slot -- so only the destructor is virtual.
/// The constructor at 0x800D2D10 builds ten dPyEffect_c from `this + 4` with
/// stride 0x13C, and `4 + 10 * 0x13C = 0xC5C` exactly. @unofficial
class dPyEffectMng_c {
public:
    /// @note Declared, NOT defined inline -- same reason as dPyEffect_c's.
    /// The real one is at 0x800D2D10 and it is what daPyMng_c's __sinit calls.
    dPyEffectMng_c();
    virtual ~dPyEffectMng_c();

    void update();
    void fn_800d2de0(float, int, mVec3_c &, u8); ///< @unofficial

    dPyEffect_c mEffects[10];

    static dPyEffectMng_c *mspInstance;
};

STATIC_ASSERT(sizeof(dPyEffectMng_c) == 0xC5C);
