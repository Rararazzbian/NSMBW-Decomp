// Probe: inline ctor + plain f32, NO initializer (just declaration)
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

BgObjName_t l_object_name[2];
