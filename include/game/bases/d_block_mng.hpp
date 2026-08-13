#pragma once

#include <types.h>

/// @brief Manager for the level's question and brick blocks.
/// @details Only the members @ref daEnBlockMain_c needs are declared so far;
/// the class layout is not yet known. @unofficial
class dBlockMng_c {
public:
    /// @brief Deletes the Yoshi that has been culled off-screen. @unofficial
    /// @return Whether one was deleted. 0x80088270.
    int YoshiDispOutDelete();

    static dBlockMng_c *m_instance;
};
