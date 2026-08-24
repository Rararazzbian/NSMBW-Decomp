#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();
        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x00) = b.x;
        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x04) = b.y;
        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x08) = b.z;
        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x0c) = rp.x;
        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x10) = rp.y;
        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x14) = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
