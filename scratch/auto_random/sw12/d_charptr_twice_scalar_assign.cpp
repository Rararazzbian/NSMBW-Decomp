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
        float ax = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        float ay = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(mPad::g_core[i]) + 0x28);
        float az = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(mPad::g_core[i]) + 0x2c);
        dst[0] = ax;
        dst[1] = ay;
        dst[2] = az;
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();
        mVec2_c raw;
        raw.set(rp.x, rp.y);
        dst[3] = raw.x;
        dst[4] = raw.y;
        dst[5] = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
