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
    float rx, ry;
    float dist;
} RandRec;

typedef struct {
    u64 time;
    char nick[0x18];
    RandRec cores[4];
} Seed;

} // namespace

u32 dRandom_c::calcMachineRandom() {
    Seed seed;
    memset(&seed, 0, sizeof(Seed));
    seed.time = OSGetTime();
    SCGetOwnerNickName(seed.nick);

    int i = 0;
    do {
        (void)0;
        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        seed.cores[i].x = b.x;
        seed.cores[i].y = b.y;
        seed.cores[i].z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        seed.cores[i].rx = raw.x;
        seed.cores[i].ry = raw.y;
        seed.cores[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(&seed, sizeof(Seed));
}
