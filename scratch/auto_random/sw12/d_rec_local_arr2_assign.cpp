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
        EGG::CoreController *cc = mPad::g_core[i];
        const f32 *fp = reinterpret_cast<const f32 *>(
            reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(fp);
        mVec3_c b(fp);
        rec->v[0] = b.x;
        rec->v[1] = b.y;
        rec->v[2] = b.z;
        EGG::Vector2f rp = cc->getDpdRawPos();
        mVec2_c raw;
        raw.set(rp.x, rp.y);
        rec->v[3] = raw.x;
        rec->v[4] = raw.y;
        rec->v[5] = cc->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
