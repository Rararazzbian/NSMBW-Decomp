#pragma once
#include <types.h>
// VERIFICATION-ONLY SHADOW COPY -- not part of the assembled.cpp deliverable
// and NOT written into include/. Real header ends at offset 0x8 (vtable ptr +
// mTimeValue); daPyMng_c::update() stb's a bool at offset 0xc, four bytes
// past the end of the real header. See BATCH3.md / ASSEMBLY.md.

class dStageTimer_c {
public:
    virtual ~dStageTimer_c() {}

    int mTimeValue;
    u32 m_8; ///< SHADOW-ONLY placeholder, not in the real header.
    bool mStopped; ///< SHADOW-ONLY placeholder @0xc, not in the real header.

    short convertToIGT() const {
        return (mTimeValue + 4095) >> 12;
    }

    static dStageTimer_c *m_instance;
};
