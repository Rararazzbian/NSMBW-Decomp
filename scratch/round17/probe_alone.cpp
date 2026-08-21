// Probe: inline ctor + plain f32 members + initializer, NO manager class
#include <types.h>

class BgObjName_t {
public:
    BgObjName_t(u32 a, u16 b, u16 c, f32 ox, f32 oy, f32 oz,
                f32 sx, f32 sy, u32 f)
        : mUnit(a), mName(b), mFlag(c), mOx(ox), mOy(oy), mOz(oz),
          mSx(sx), mSy(sy), mParam(f) {}
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
    BgObjName_t(0x35, 0x18E, 0x0, 8.0f, -8.0f, 0.0f, 32.0f, 32.0f, 0x0),
    BgObjName_t(0x0, 0x2EB, 0x0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0x0),
};
