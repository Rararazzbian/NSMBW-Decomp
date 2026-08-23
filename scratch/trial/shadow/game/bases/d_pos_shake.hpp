#pragma once
#include <types.h>

/// @brief Screen-shake helper: integrates a shake velocity towards rest and
/// applies the resulting offset.
/// @details All member types, the class size and the argument->member mapping
/// of init() are derived from the retail disassembly of this TU
/// (init/move/startShake, 0x800D81A0-0x800D82D0); see TRIAL_RESPONSE.md.
/// Names are placeholders derived from offsets. No vtable: none of the three
/// functions performs virtual dispatch and the TU emits no .data.
/// @unofficial
class dPosShake_c {
public:
    void init(f32, f32, f32, f32, f32, f32);
    void move();
    void startShake(f32);

    /* 0x00 */ /// @brief Current shake offset. @unofficial
    f32 mOffset;
    /* 0x04 */ /// @brief Shake velocity; also what startShake() adds to. @unofficial
    f32 mVel;
    /* 0x08 */ /// @brief Scales the offset when updating the velocity. @unofficial
    f32 mK;
    /* 0x0C */ /// @brief Step size pulling the velocity toward zero. @unofficial
    f32 mDamp;
    /* 0x10 */ /// @brief Snap threshold reused as the snap step. @unofficial
    f32 mSnap;
    /* 0x14 */ /// @brief Velocity clamp bound. @unofficial
    f32 mLimit;
}; // Size 0x18

STATIC_ASSERT(sizeof(dPosShake_c) == 0x18);
