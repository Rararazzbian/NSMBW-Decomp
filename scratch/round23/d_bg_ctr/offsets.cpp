#include <game/bases/d_bg_ctr.hpp>

// compile-time offset checks; a mismatch fails the build
#define CHK(member, off) \
    typedef char chk_##member[(offsetof(dBg_ctr_c, member) == (off)) ? 1 : -1]

CHK(mpActor, 0x00);
CHK(mCenter, 0x80);
CHK(mOffset2, 0x88);
CHK(mRadius, 0x90);
CHK(m_a0, 0xa0);
CHK(m_a8, 0xa8);
CHK(m_ac, 0xb0);
CHK(mRotation, 0xbc);
CHK(m_c0, 0xc0);
CHK(m_c2, 0xc2);
CHK(m_c4, 0xc4);
CHK(mMode, 0xc8);
CHK(mFlags2, 0xcc);
CHK(mFlags, 0xd0);
CHK(m_d4, 0xd4);
CHK(mEntryFlag, 0xdc);
CHK(m_dd, 0xdd);
CHK(m_de, 0xde);
CHK(mGroupNo, 0xe0);

typedef char chk_size[sizeof(dBg_ctr_c) == 0xe4 ? 1 : -1];
