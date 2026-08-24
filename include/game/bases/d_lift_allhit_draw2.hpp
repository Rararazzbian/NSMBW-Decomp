#pragma once
#include <types.h>

/// Lift "all-hit" collision box renderer (second variant).
///
/// Only ::draw is a named retail symbol for this class; the object is never
/// constructed or called anywhere in wiimj2d.dol, so the layout below covers
/// exactly the offsets draw() reads. The two large regions at +0x1c and
/// +0x22ac are same-size (0x2290) blocks handed to the shared lift-draw
/// helper; their internal structure is not yet known.
class dLiftAllhitDraw2_c {
public:
    void draw();

    u8 mPad0[0x1c];
    u8 mUnk1c[0x2290];
    u8 mUnk22ac[0x2290];
    float mMinY;    // 0x453c
    float mMidY;    // 0x4540
    float mMaxY;    // 0x4544
    s16 mAng;       // 0x4548
    u8 mColA;       // 0x454a
    u8 mColB;       // 0x454b
    u8 mColC;       // 0x454c
};
