#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {

struct Collector {
    static void collectOne(float *dst, int slot, int i) {
        EGG::CoreController *cc = mPad::g_core[i];
        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector2f rp = cc->getDpdRawPos();

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;

        int o = slot * 6;
        dst[o + 0] = b.x;
        dst[o + 1] = b.y;
        dst[o + 2] = b.z;
        dst[o + 3] = raw.x;
        dst[o + 4] = raw.y;
        dst[5] = mPad::g_core[i]->getDpdDistance();
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
        Collector::collectOne(reinterpret_cast<float *>(&buf[0x20]), i, i);
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
