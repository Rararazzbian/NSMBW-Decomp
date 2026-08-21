// Probe: does a typedef alias of a nested type register arrays?
#include <types.h>
#include <game/mLib/m_vec.hpp>

class Outer_t {
public:
    struct BgObjName_t {
        BgObjName_t(u32 a, u16 b, u16 c, mVec3_c off, mVec2_c size, u32 f);
        ~BgObjName_t();
        u32 mUnit;
        u16 mName;
        u16 mFlag;
        mVec3_c mOffset;
        mVec2_c mSize;
        u32 mParam;
    };
};

Outer_t::BgObjName_t::BgObjName_t(u32 a, u16 b, u16 c, mVec3_c off, mVec2_c size,
                                  u32 f)
    : mUnit(a), mName(b), mFlag(c), mOffset(off), mSize(size), mParam(f) {}
Outer_t::BgObjName_t::~BgObjName_t() {}

typedef Outer_t::BgObjName_t BgObjName_t;

BgObjName_t l_alias[2] = {
    BgObjName_t(0x35, 0x18E, 0x0, mVec3_c(8.0f, -8.0f, 0.0f), mVec2_c(32.0f, 32.0f), 0x0),
    BgObjName_t(0x0, 0x2EB, 0x0, mVec3_c(0.0f, 0.0f, 0.0f), mVec2_c(0.0f, 0.0f), 0x0),
};
