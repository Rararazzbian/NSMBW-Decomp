#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);


namespace {
struct RandRec { float x, y, z, rx, ry, dist; };
struct RandBlock {
    s64 time;
    char nick[0x18];
    RandRec recs[4];
};
}

u32 dRandom_c::calcMachineRandom() {
    union {
        char bytes[0x80];
        RandBlock d;
    } u;

    memset(u.bytes, 0, 0x80);
    u.d.time = OSGetTime();
    SCGetOwnerNickName(u.d.nick);

    int i = 0;
    do {
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();

        u.d.recs[i].x = b.x;
        u.d.recs[i].y = b.y;
        u.d.recs[i].z = b.z;
        u.d.recs[i].rx = rp.x;
        u.d.recs[i].ry = rp.y;
        u.d.recs[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(u.bytes, 0x80);
}
