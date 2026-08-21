// Probe: AGGREGATE-style partial initializer + user dtor (no ctor at all)
#include <types.h>

class BgObjName_t {
public:
    ~BgObjName_t();
    u32 mUnit;
    u16 mName;
    u16 mFlag;
    f32 mOx, mOy, mOz;
    f32 mSx, mSy;
    u32 mParam;
};

BgObjName_t::~BgObjName_t() {}

BgObjName_t l_object_name[2] = {
    {0x35, 0x18E, 0x0, 8.0f, -8.0f, 0.0f, 32.0f, 32.0f, 0x0},
    {0x0, 0x2EB, 0x0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0x0},
};
