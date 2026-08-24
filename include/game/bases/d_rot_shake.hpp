#pragma once
#include <types.h>

/**
 * @brief Rotation screen-shake helper.
 * @details Companion to the positional screen-shake helper. Holds a value that
 * decays toward zero every frame while nudging a secondary counter; members
 * are all 16-bit except one int flag. No virtual functions, no base class,
 * no destructor exists in the retail unit (the TU contains exactly these two
 * functions).
 * @ingroup bases
 * @unofficial
 */
class dRotShake_c {
public:
    /// Stores the eight incoming halfwords into members 0x0-0xf (scrambled
    /// mapping; see d_rot_shake.cpp). @unofficial
    void init(s16 p1, s16 p2, s16 p3, s16 p4, s16 p5, s16 p6, s16 p7, s16 p8);

    /// Updates the shake state. Returns the new m002 value. @unofficial
    s16 move();

    // @unofficial -- offset placeholder names; layout derived from the
    // init__11dRotShake_cFssssssss / move__11dRotShake_cFv disassembly.
    s16 m000; ///< 0x00. Decay accumulator approaching 0 (init arg 6).
    s16 m002; ///< 0x02. Nudged secondary value; move() returns it (init arg 5).
    s16 m004; ///< 0x04. Divisor applied to -m002 (init arg 2).
    s16 m006; ///< 0x06. Per-frame decay step (init arg 3).
    s16 m008; ///< 0x08. Clamp limit for the m000 update (init arg 4).
    s16 m00a; ///< 0x0A. Dead-zone threshold governing m002 vs m000 (init arg 7).
    s16 m00c; ///< 0x0C. Snap threshold for the m002 nudge (init arg 8).
    s16 m00e; ///< 0x0E. Written by init only; never read here (init arg 1).
    int m010; ///< 0x10. Flag, set when m002 snaps to 0 in move().
};

STATIC_ASSERT(sizeof(dRotShake_c) == 0x14);
