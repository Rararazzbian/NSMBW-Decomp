#pragma once
#include <types.h>

class dStageTimer_c {
public:
    virtual ~dStageTimer_c() {}

    int mTimeValue;                 ///< [+0x04] Fixed-point countdown timer (IGT << 12).
    s16 mStartTimer;                ///< [+0x08] Initial time value in seconds. @unofficial
    u8 mTimeUp;                     ///< [+0x0A] Time up flag. @unofficial
    u8 mHurryUpSoundPlayed;         ///< [+0x0B] Hurry up sound played flag (time <= 100). @unofficial
    bool mStopped;                  ///< [+0x0C] Timer freeze/stopped flag. Written by daPyMng_c::update().
    u8 mPad0D[3];                   ///< [+0x0D] Padding to 0x10. @unofficial

    short convertToIGT() const {
        return (mTimeValue + 4095) >> 12;
    }

    static dStageTimer_c *m_instance;
};
