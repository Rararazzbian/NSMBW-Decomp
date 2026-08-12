#pragma once
#include <types.h>

/// @brief Tracks the state of the level's event (switch) flags.
/// @details Placeholder — the manager's own translation unit is not decompiled
/// yet, so only the members reached from decompiled code are named. The
/// remaining space is padding of the verified total size.
/// @unofficial
/// @ingroup bases
class dSwitchFlagMng_c {
public:
    u64 mFlags;               ///< The currently active event flags.
    u8 mPad[0x6c8 - 0x8];
    u8 mPlayerNo;             ///< The player who last triggered a flag. @unofficial

    static dSwitchFlagMng_c *m_instance; ///< The manager instance.
};
