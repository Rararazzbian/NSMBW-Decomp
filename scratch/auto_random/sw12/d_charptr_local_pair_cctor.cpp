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
        float *dst = reinterpret_cast<float *>(&buf[0x20 + i * 0x18]);
        EGG::CoreController *cc = mPad::g_core[i];
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        dst[0] = b.x;
        dst[1] = b.y;
        dst[2] = b.z;
        EGG::Vector2f rp = cc->getDpdRawPos();
        mVec2_c raw(*reinterpret_cast<const mVec2_c *>(&rp));
        dst[3] = raw.x;
        dst[4] = raw.y;
        dst[5] = cc->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
