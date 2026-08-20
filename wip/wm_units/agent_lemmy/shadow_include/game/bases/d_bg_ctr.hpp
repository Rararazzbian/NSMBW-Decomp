#pragma once

#include <game/bases/d_actor.hpp>

// @unofficial PROPOSED HEADER CHANGE. `dBg_ctr_c::set()` has a THIRD
// overload the landed header is missing, confirmed two ways: (1) the
// target LEMMY_FOOTHOLD/LEMMY_FOOTHOLD_MAIN both call
// `set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c` directly, a
// mangled name matching NEITHER of the two overloads already declared
// below; (2) the DOL's own map confirms a real, compiled definition at
// `.text:0x8007FB10` (size 0x64) under that exact mangled name -- this
// is not a guess, the function exists. `sBgSetInfo` itself is also real
// (`setObjBgInfoCallBack__17daObjMoveOnBase_cFR10sBgSetInfo` at
// `0x800454C0` takes it by reference) but is declared nowhere in the
// tree. A forward declaration is sufficient here -- the parameter is a
// pointer, so the layout is not needed to declare the method, and no
// fields are invented. Parameters are proven by the mangling
// (`dActor_c*, const sBgSetInfo*, u8, u8, mVec3_c*`); the return type
// is the one open question with CFront-style mangling, and it was
// checked, not assumed: at both of this unit's own call sites, `r3`
// (the return value) is immediately clobbered by the next instruction's
// own `addi`/`lwz` before ever being read -- the result is provably
// unused at both sites, matching the other two `set()` overloads' own
// `void` return (suggestive on its own, but the clobber pattern is the
// actual evidence here).
struct sBgSetInfo;

class dBg_ctr_c {
public:
    dActor_c *mpActor;
    u8 mPad1[0x9c];
    mVec2_c m_a0;
    mVec2_c m_ac;
    u8 mPad2[0xc];
    short *mRotation;
    short m_c0;
    short m_c2;
    u8 mPad4[0x4];
    int m_c8;
    u32 mFlags2;
    int mFlags;
    int m_d4;
    u8 mpPad5[0x4];
    bool m_dc;
    u8 mpPad6[0x3];
    int m_e0;
    // u8 m_e1;
    // u8 m_e2;

    typedef void CallbackF(dActor_c *self, dActor_c *other);
    typedef void CallbackH(dActor_c *self, dActor_c *other);
    typedef void CallbackW(dActor_c *self, dActor_c *other, u8);

    dBg_ctr_c();
    ~dBg_ctr_c();

    void entry();
    void release();
    void set(dActor_c*, float, float, float, float, CallbackF *, CallbackH *, CallbackW *, u8, u8, mVec3_c *);
    void set(dActor_c*, mVec2_c, mVec2_c, CallbackF *, CallbackH *, CallbackW *, u8, u8, mVec3_c *);
    void set(dActor_c *, const sBgSetInfo *, u8, u8, mVec3_c *);
    void setOfs(float, float, float, float, mVec3_c *);
    void calc();

    void addDokanMoveDiff(mVec3_c *);
};
