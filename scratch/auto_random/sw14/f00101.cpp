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
    for (int i = 0; i < 4; i++) {
        EGG::CoreController *cc = mPad::g_core[i];
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        mVec2_c raw;
        EGG::Vector2f rp = cc->getDpdRawPos();
        float *dst = reinterpret_cast<float *>(buf + 0x20) + i * 6;
        dst[0] = b.x;
        dst[1] = b.y;
        dst[2] = b.z;
        raw.set(rp.x, rp.y);
        dst[3] = rp.x;
        dst[4] = rp.y;
        dst[5] = cc->getDpdDistance();
    }
    return OSCalcCRC32(buf, 0x80);
}
