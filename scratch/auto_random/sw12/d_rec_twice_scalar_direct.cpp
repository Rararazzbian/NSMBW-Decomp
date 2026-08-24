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
        struct Rec { float v[6]; };
        Rec *rec = reinterpret_cast<Rec *>(buf + 0x20) + i;
        float ax = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        float ay = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(mPad::g_core[i]) + 0x28);
        float az = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(mPad::g_core[i]) + 0x2c);
        rec->v[0] = ax;
        rec->v[1] = ay;
        rec->v[2] = az;
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();
        rec->v[3] = rp.x;
        rec->v[4] = rp.y;
        rec->v[5] = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
