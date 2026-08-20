#pragma once

#include <game/bases/d_actor.hpp>

// SHADOW COPY of the real, landed include/game/bases/d_bg_ctr.hpp, adding
// ONE overload of set() that the real header does not yet declare.
//
// FLOOR_JR_A's own fn_2_83970 calls a THIRD set() overload, confirmed by its
// own mangled call target: `set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c`
// = `set(dActor_c*, const sBgSetInfo*, u8, u8, mVec3_c*)` -- neither of the two
// overloads the real header already declares (float-args or mVec2_c-args, both
// taking three callback pointers) matches this signature: this one takes a
// `const sBgSetInfo*` and NO callback pointers at all. `sBgSetInfo`'s own shape
// read directly off fn_2_83970's own stack layout (a local built at
// `addi r5, r1, 0x14` immediately before the call): four floats then three
// zeroed words, 0x1c bytes total.
struct sBgSetInfo {
    float mLeft;
    float mTop;
    float mRight;
    float mBottom;
    int m_10;
    int m_14;
    int m_18;
};

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
    void set(dActor_c*, const sBgSetInfo*, u8, u8, mVec3_c *);
    void setOfs(float, float, float, float, mVec3_c *);
    void calc();

    void addDokanMoveDiff(mVec3_c *);
};
