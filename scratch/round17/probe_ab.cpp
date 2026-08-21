// Minimal A/B probe: which source shape triggers __register_global_object?
// Shape A: 6-arg ctor with initializer-list array (our current draft)
// Shape B: default ctor, no-initializer array (m_pad shape)
#include <types.h>
#include <game/mLib/m_vec.hpp>

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

ShapeA_t::ShapeA_t(u32 a, u16 b, u16 c, mVec3_c off, mVec2_c size, u32 f)
    : mUnit(a), mName(b), mFlag(c), mOffset(off), mSize(size), mParam(f) {}
ShapeA_t::~ShapeA_t() {}

ShapeA_t l_shapeA[2] = {
    ShapeA_t(0x35, 0x18E, 0x0, mVec3_c(8.0f, -8.0f, 0.0f), mVec2_c(32.0f, 32.0f), 0x0),
    ShapeA_t(0x0, 0x2EB, 0x0, mVec3_c(0.0f, 0.0f, 0.0f), mVec2_c(0.0f, 0.0f), 0x0),
};

class ShapeB_t {
public:
    ShapeB_t();
    ~ShapeB_t();
    u32 mUnit;
};

ShapeB_t::ShapeB_t() { mUnit = 0; }
ShapeB_t::~ShapeB_t() {}

ShapeB_t l_shapeB[2];
