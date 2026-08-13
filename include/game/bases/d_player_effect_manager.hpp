#pragma once

#include <game/mLib/m_vec.hpp>

/// @brief One player-linked effect slot.
/// @details sizeof 0x13C, fixed by dPyEffectMng_c's array stride.
/// `__vt__11dPyEffect_c` is 0xC -- one virtual slot -- so the destructor is the
/// only virtual function and `update()` is NOT virtual, despite existing as
/// `update__11dPyEffect_cFv`. The members below `0x118` are not yet
/// reconstructed; the pad holds their space. @unofficial
class dPyEffect_c {
public:
    virtual ~dPyEffect_c();

    void update();

    /// @brief [0x004..0x13C] Not yet reconstructed. Known from
    /// `fn_800D2BB0`: position floats at 0x118/0x11C/0x120, scale floats at
    /// 0x124/0x128/0x12C, a layer byte at 0x130, an effect id at 0x134 and an
    /// active flag at 0x138.
    u8 pad4[0x138];
};

STATIC_ASSERT(sizeof(dPyEffect_c) == 0x13C);

/// @brief Owns the ten player-linked effect slots.
/// @details [.bss instance at 0x80355354, sizeof 0xC5C]. `__vt__14dPyEffectMng_c`
/// is 0xC -- one virtual slot -- so only the destructor is virtual.
/// The constructor at 0x800D2D10 builds ten dPyEffect_c from `this + 4` with
/// stride 0x13C, and `4 + 10 * 0x13C = 0xC5C` exactly. @unofficial
class dPyEffectMng_c {
public:
    virtual ~dPyEffectMng_c();

    void update();
    void fn_800d2de0(float, int, mVec3_c &, u8); ///< @unofficial

    dPyEffect_c mEffects[10];

    static dPyEffectMng_c *mspInstance;
};

STATIC_ASSERT(sizeof(dPyEffectMng_c) == 0xC5C);
