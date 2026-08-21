// Probe: 4 top-level arrays + a manager class with a MEMBER of the array type
#include <types.h>
#include <game/mLib/m_vec.hpp>

class dBgActorManager_c {
public:
    struct BgObjName_t;
    dBgActorManager_c();
    virtual ~dBgActorManager_c();
};

struct dBgActorManager_c::BgObjName_t {
    BgObjName_t(u32 a, u16 b, u16 c, mVec3_c off, mVec2_c size, u32 f);
    ~BgObjName_t();
    u32 mUnit;
    u16 mName;
    u16 mFlag;
    mVec3_c mOffset;
    mVec2_c mSize;
    u32 mParam;
};

dBgActorManager_c::BgObjName_t::BgObjName_t(u32 a, u16 b, u16 c, mVec3_c off,
                                            mVec2_c size, u32 f)
    : mUnit(a), mName(b), mFlag(c), mOffset(off), mSize(size), mParam(f) {}
dBgActorManager_c::BgObjName_t::~BgObjName_t() {}
dBgActorManager_c::dBgActorManager_c() {}
dBgActorManager_c::~dBgActorManager_c() {}

typedef dBgActorManager_c::BgObjName_t BgObjName_t;

BgObjName_t l_object_name[2] = {
    BgObjName_t(0x35, 0x18E, 0x0, mVec3_c(8.0f, -8.0f, 0.0f), mVec2_c(32.0f, 32.0f), 0x0),
    BgObjName_t(0x0, 0x2EB, 0x0, mVec3_c(0.0f, 0.0f, 0.0f), mVec2_c(0.0f, 0.0f), 0x0),
};
BgObjName_t l_Pa3_rail[2] = {
    BgObjName_t(0x35, 0x18E, 0x0, mVec3_c(8.0f, -8.0f, 0.0f), mVec2_c(32.0f, 32.0f), 0x0),
    BgObjName_t(0x0, 0x2EB, 0x0, mVec3_c(0.0f, 0.0f, 0.0f), mVec2_c(0.0f, 0.0f), 0x0),
};
