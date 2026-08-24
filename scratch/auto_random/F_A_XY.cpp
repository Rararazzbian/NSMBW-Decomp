#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {
struct RandRec { float x, y, z; mVec2_c raw; float dist; };
typedef RandRec RandRecArr[4];
}

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);
    mVec3_c a;
    mVec3_c b;
    mVec2_c raw;
    int i = 0;

    do {
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        a = pos;
        b = a;
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        recs[i].x = b.x;
        recs[i].y = b.y;
        recs[i].z = b.z;
        recs[i].raw = raw;
        raw.x = rp.x;
        raw.y = rp.y;
        recs[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
