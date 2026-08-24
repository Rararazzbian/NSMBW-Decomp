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

    mVec3_c a;
    mVec3_c b;
    mVec2_c raw;
    int i = 0;
    int coreOff = 0;
    int recOff = 0x20;

    do {
        EGG::CoreController *cc = *reinterpret_cast<EGG::CoreController *const *>(
            (const char *)mPad::g_core + coreOff);
        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(reinterpret_cast<const char *>(cc) + 0x24);
        a = pos;
        b = a;
        EGG::Vector2f rp = cc->getDpdRawPos();

        *(float *)&buf[recOff + 0x00] = b.x;
        *(float *)&buf[recOff + 0x04] = b.y;
        *(float *)&buf[recOff + 0x08] = b.z;
        raw.x = rp.x;
        raw.y = rp.y;
        *(mVec2_c *)&buf[recOff + 0x0c] = raw;
        *(float *)&buf[recOff + 0x14] = cc->getDpdDistance();

        coreOff += 4;
        recOff += 0x18;
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
