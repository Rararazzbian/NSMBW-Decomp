#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {

typedef struct {
    float x, y, z;
    mVec2_c raw;
    float dist;
} RandRec;

} // namespace

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
        RandRec &rec = recs[i];
        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        rec.x = b.x;
        rec.y = b.y;
        rec.z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;
        rec.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
