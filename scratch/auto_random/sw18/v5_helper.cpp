#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {

struct RandRec {
    float x, y, z;
    mVec2_c raw;
    float dist;
};

typedef RandRec RandRecArr[4];

struct Collector {
    static void collectOne(float *dstBase, int i) {
        EGG::CoreController *cc = mPad::g_core[i];
        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        dstBase[i * 6 + 0] = b.x;
        dstBase[i * 6 + 1] = b.y;
        dstBase[i * 6 + 2] = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        dstBase[i * 6 + 3] = raw.x;
        dstBase[i * 6 + 4] = raw.y;

        dstBase[i * 6 + 5] = mPad::g_core[i]->getDpdDistance();
    }
};

} // namespace

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
        Collector::collectOne(reinterpret_cast<float *>(&buf[0x20]), i);
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
