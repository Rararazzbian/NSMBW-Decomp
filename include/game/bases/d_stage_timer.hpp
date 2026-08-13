#pragma once
#include <types.h>

class dStageTimer_c {
public:
    virtual ~dStageTimer_c() {}

    int mTimeValue;         ///< [+0x04] Countdown, IGT << 12.
    s16 mStartTimer;        ///< [+0x08] Initial time in seconds. @unofficial
    u8 mTimeUp;             ///< [+0x0A] @unofficial
    u8 mHurryUpSoundPlayed; ///< [+0x0B] @unofficial
    /// @brief [+0x0C] Timer freeze flag, written by daPyMng_c::update().
    /// Constructor at 0x800E38E0 stores it with `stb r0, 0xc(r3)`, and
    /// createInstance allocates 0x10, which pins the whole layout. @unofficial
    bool mStopped;
    u8 mPad0D[3];           ///< [+0x0D] Pad to 0x10. @unofficial

    short convertToIGT() const {
        return (mTimeValue + 4095) >> 12;
    }

    static dStageTimer_c *m_instance;
};
