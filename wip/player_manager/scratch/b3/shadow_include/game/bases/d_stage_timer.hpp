#pragma once
#include <types.h>

class dStageTimer_c {
public:
    virtual ~dStageTimer_c() {}

    int mTimeValue;
    u32 m_8; ///< SHADOW-ONLY placeholder, not in the real header. See BATCH3.md.
    bool mStopped; ///< SHADOW-ONLY placeholder @0xc, not in the real header. See BATCH3.md.

    short convertToIGT() const {
        return (mTimeValue + 4095) >> 12;
    }

    static dStageTimer_c *m_instance;
};
