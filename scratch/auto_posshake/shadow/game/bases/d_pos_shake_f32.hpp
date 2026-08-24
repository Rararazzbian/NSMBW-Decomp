#pragma once
#include <types.h>

/// Variant header: move() declared as returning f32 (mangling cannot
/// distinguish; only register allocation can). @unofficial
class dPosShake_c {
public:
    void init(f32, f32, f32, f32, f32, f32);
    f32 move();
    void startShake(f32);

    /* 0x00 */ f32 mOffset; // @unofficial
    /* 0x04 */ f32 mVel;    // @unofficial
    /* 0x08 */ f32 mK;      // @unofficial
    /* 0x0C */ f32 mDamp;   // @unofficial
    /* 0x10 */ f32 mSnap;   // @unofficial
    /* 0x14 */ f32 mLimit;  // @unofficial
}; // Size 0x18

STATIC_ASSERT(sizeof(dPosShake_c) == 0x18);
