#include <game/bases/d_ice_manager.hpp>
#include <game/bases/d_game_com.hpp>

class dIceEfMaker_c : public dActor_c {
public:
    void init(int, dIceEfScale_c *);
    int execute();
    void fin();
    void setEfScale(const dIceEfScale_c &);
    void createEffect(int);
    void hahenEffect();

    u8 mData[0x848 - sizeof(dActor_c)];
};

static dIceEfScale_c l_mdl_scale_tbl[8];

static inline u32 &flags(dIceEfMaker_c *p) { return *(u32 *)((u8 *)p + 0x0); }
static inline int &mode(dIceEfMaker_c *p) { return *(int *)((u8 *)p + 0x4); }
static inline float &field(dIceEfMaker_c *p, int offset) { return *(float *)((u8 *)p + offset); }
static inline dActor_c *owner(dIceEfMaker_c *p) { return *(dActor_c **)((u8 *)p + 0x848); }
static inline dActor_c *effect(dIceEfMaker_c *p, int i) { return *(dActor_c **)((u8 *)p + 0x828 + i * 4); }

void dIceEfMaker_c::init(int kind, dIceEfScale_c *scale) {
    flags(this) = 0;
    mode(this) = kind;
    if (scale == nullptr) {
        scale = &l_mdl_scale_tbl[kind];
    }
    setEfScale(*scale);
}

int dIceEfMaker_c::execute() {
    mVec3_c pos;
    pos = owner(this)->getCenterPos();
    pos.z = 0.0f;
    for (int i = 0; i < 8; i++) {
        u32 mask = 1 << i;
        if (flags(this) & mask) {
            dActor_c *p = effect(this, i);
            typedef int (*Fn)(dActor_c *, const mVec3_c &);
            Fn fn = (Fn)(*(u32 **)(*(u32 **)p + 0xc));
            if (fn(p, pos) == 0) {
                flags(this) &= ~mask;
            }
        }
    }
    return 0;
}

void dIceEfMaker_c::fin() {}

void dIceEfMaker_c::setEfScale(const dIceEfScale_c &scale) {
    field(this, 0x0c) = scale.mData[0]; field(this, 0x10) = scale.mData[0]; field(this, 0x14) = scale.mData[0];
    field(this, 0x130) = scale.mData[1]; field(this, 0x134) = scale.mData[1]; field(this, 0x138) = scale.mData[1];
    field(this, 0x268) = scale.mData[2]; field(this, 0x26c) = scale.mData[2]; field(this, 0x270) = scale.mData[2];
    field(this, 0x38c) = scale.mData[3]; field(this, 0x390) = scale.mData[3]; field(this, 0x394) = scale.mData[3];
    field(this, 0x4b0) = scale.mData[4]; field(this, 0x4b4) = scale.mData[4]; field(this, 0x4b8) = scale.mData[4];
    field(this, 0x5d4) = scale.mData[5]; field(this, 0x5d8) = scale.mData[5]; field(this, 0x5dc) = scale.mData[5];
    field(this, 0x6f8) = scale.mData[6]; field(this, 0x6fc) = scale.mData[6]; field(this, 0x700) = scale.mData[6];
    field(this, 0x81c) = scale.mData[7]; field(this, 0x820) = scale.mData[7]; field(this, 0x824) = scale.mData[7];
}

void dIceEfMaker_c::createEffect(int kind) {
    mVec3_c pos;
    pos = owner(this)->getCenterPos();
    pos.z = 0.0f;
    dActor_c *p = effect(this, kind);
    typedef void (*Fn)(dActor_c *, const mVec3_c &);
    Fn fn = (Fn)(*(u32 **)(*(u32 **)p + 0x8));
    fn(p, pos);
    flags(this) |= 1 << kind;
}

void dIceEfMaker_c::hahenEffect() {
    mVec3_c pos;
    pos = owner(this)->getCenterPos();
    pos.z = 0.0f;
    int kind = dGameCom::rndInt(4);
    if (mode(this) >= 3) kind |= 0x10;
    typedef void (*Fn)(u32, mVec3_c &, u32, s8);
    Fn fn = (Fn)*(u32 *)0x8042A1B0;
    fn(0, pos, kind, -1);
}
