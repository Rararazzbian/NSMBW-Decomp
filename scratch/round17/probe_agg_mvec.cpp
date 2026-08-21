// Probe: AGGREGATE with mVec3_c/mVec2_c members + user dtor
#include <types.h>
#include <game/mLib/m_vec.hpp>

class dBgActorManager_c {
public:
    struct BgObjName_t {
        ~BgObjName_t();
        u32 mUnit;
        u16 mName;
        u16 mFlag;
        mVec3_c mOffset;
        mVec2_c mSize;
        u32 mParam;
    };
};

dBgActorManager_c::BgObjName_t::~BgObjName_t() {}

dBgActorManager_c::BgObjName_t l_object_name[2] = {
    {0x35, 0x18E, 0x0, {8.0f, -8.0f, 0.0f}, {32.0f, 32.0f}, 0x0},
    {0x0, 0x2EB, 0x0, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, 0x0},
};
