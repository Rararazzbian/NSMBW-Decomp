#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace { struct RandRec { float x, y, z; mVec2_c raw; float dist; }; }


u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    int i = 0;
    do {
        float *dst = reinterpret_cast<float *>(&buf[0x20]) + i * 6;
        EGG::CoreController *cc = mPad::g_core[i];
        dst[0] = (float)cc->getKey(0);
        dst[5] = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
