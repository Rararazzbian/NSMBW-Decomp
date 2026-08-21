// Probe: identical struct, ONE named `BgObjName_t` (nested), ONE `ShapeA_t`
#include <types.h>
#include <game/mLib/m_vec.hpp>

class dBgActorManager_c {
public:
    struct BgObjName_t;
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

class ShapeA_t {
public:
    ShapeA_t(u32 a, u16 b, u16 c, mVec3_c off, mVec2_c size, u32 f);
    ~ShapeA_t();
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

ShapeA_t::ShapeA_t(u32 a, u16 b, u16 c, mVec3_c off, mVec2_c size, u32 f)
    : mUnit(a), mName(b), mFlag(c), mOffset(off), mSize(size), mParam(f) {}
ShapeA_t::~ShapeA_t() {}

dBgActorManager_c::BgObjName_t l_nested[2] = {
    dBgActorManager_c::BgObjName_t(0x35, 0x18E, 0x0, mVec3_c(8.0f, -8.0f, 0.0f), mVec2_c(32.0f, 32.0f), 0x0),
    dBgActorManager_c::BgObjName_t(0x0, 0x2EB, 0x0, mVec3_c(0.0f, 0.0f, 0.0f), mVec2_c(0.0f, 0.0f), 0x0),
};

ShapeA_t l_top[2] = {
    ShapeA_t(0x35, 0x18E, 0x0, mVec3_c(8.0f, -8.0f, 0.0f), mVec2_c(32.0f, 32.0f), 0x0),
    ShapeA_t(0x0, 0x2EB, 0x0, mVec3_c(0.0f, 0.0f, 0.0f), mVec2_c(0.0f, 0.0f), 0x0),
};
