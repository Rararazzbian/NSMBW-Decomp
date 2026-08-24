#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {
struct Rec { float x, y, z, rx, ry, dist; };
}


u32 dRandom_c::calcMachineRandom() {
    int i = 0;
    Rec recs[4];
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    do {
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();

        recs[i].x = b.x;
        recs[i].y = b.y;
        recs[i].z = b.z;
        recs[i].rx = rp.x;
        recs[i].ry = rp.y;
        recs[i].dist = mPad::g_core[i]->getDpdDistance();
    }
 while (++i < 4);
    return OSCalcCRC32(buf, 0x80);
}
