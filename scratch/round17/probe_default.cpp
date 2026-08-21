// Probe: default ctor + dtor, no initializer (m_pad shape)
#include <types.h>

class BgObjName_t {
public:
    BgObjName_t() : mUnit(0), mName(0), mFlag(0), mOx(0), mOy(0), mOz(0),
                    mSx(0), mSy(0), mParam(0) {}
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
