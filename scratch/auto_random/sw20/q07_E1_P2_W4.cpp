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

struct SeedData {
    u64 time;
    char nick[0x18];
    RandRec cores[4];
};

} // namespace

u32 dRandom_c::calcMachineRandom() {
    SeedData data;
    memset(&data, 0, sizeof(SeedData));
    data.time = OSGetTime();
    SCGetOwnerNickName(data.nick);

    int i = 0;
    do {
        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        data.cores[i].x = b.x;
        data.cores[i].y = b.y;
        data.cores[i].z = b.z;

        data.cores[i].raw.x = rp.x;
        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        data.cores[i].raw.y = rp.y;
        data.cores[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(&data, sizeof(SeedData));
}
