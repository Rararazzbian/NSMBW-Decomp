#pragma once

class dBg_ctr_c;  // d_bc.hpp references it before we are fully defined

#include <game/bases/d_actor.hpp>
#include <game/bases/d_bc.hpp>
#include <game/mLib/m_vec.hpp>

// @unofficial SHADOW COPY for scratch/round22/d_bg_ctr/draft.cpp
// Layout derived from the d_bg_ctr_c target disassembly (ctor/dtor array at
// 0x60, init clears 0x4..0x34 + 0xDC + 0xE0, set_common writes 0x40..0x54 +
// 0xDD/0xDE, link slots at 0x18..0x24 / 0x28..0x2C, calc reads 0xA0/0xA8/0xB0).

// floats at +0/4/8/0xC, callbacks at +0x10/0x14/0x18 (from set(sBgSetInfo))
struct sBgSetInfo {
    f32 f0, f4, f8, fC;
    void *cbF, *cbH, *cbW;
};

class dBg_ctr_c {
public:
    typedef void CallbackF(dActor_c *self, dActor_c *other);
    typedef void CallbackH(dActor_c *self, dActor_c *other);
    typedef void CallbackW(dActor_c *self, dActor_c *other, u8);
    typedef bool CheckRevF(dActor_c *self, dActor_c *other);
    typedef bool CheckRevW(dActor_c *self, dActor_c *other, u8);

    dActor_c *mpActor;          // 0x00
    dBg_ctr_c *mEntryPrev;      // 0x04
    dBg_ctr_c *mEntryNext;      // 0x08
    int m_0c;                   // 0x0c
    int m_10;                   // 0x10
    int m_14;                   // 0x14
    dBc_c *mLinkNetPlayer[4];   // 0x18..0x24
    dBc_c *mWallSlidPlayer[4];  // 0x28..0x34
    int m_38;                   // 0x38 (uninitialized by init)
    int mUpdateFlag;            // 0x3c (calc sets 1, update clears)
    CallbackF *mCallbackF;      // 0x40
    CallbackH *mCallbackH;      // 0x44
    CallbackW *mCallbackW;      // 0x48
    CheckRevF *mCheckRevUpper;  // 0x4c
    CheckRevF *mCheckRevUnder;  // 0x50
    CheckRevW *mCheckRevSide;   // 0x54
    mVec2_c mPos;               // 0x58
    mVec2_c mScratch[4];        // 0x60 (ctor/dtor-constructed array)
    mVec2_c mCenter;            // 0x80 (rect: offset1 / circle: center)
    mVec2_c mOffset2;           // 0x88
    f32 mRadius;                // 0x90 (circle)
    u8 mPad94[0xc];             // 0x94
    mVec2_c m_a0;               // 0xa0
    mVec2_c m_a8;               // 0xa8
    mVec2_c m_ac;               // 0xb0
    u8 mPadB8[4];               // 0xb8 (never touched by this unit's code)
    short *mRotation;           // 0xbc
    short m_c0;                 // 0xc0
    short m_c2;                 // 0xc2
    short m_c4;                 // 0xc4
    u8 mPadC6[0x2];             // 0xc6
    int mMode;                  // 0xc8 (0 rect, 1 circle)
    u32 mFlags2;                // 0xcc
    int mFlags;                 // 0xd0
    int m_d4;                   // 0xd4
    int m_d8;                   // 0xd8 (update masks to bit0)
    bool mEntryFlag;            // 0xdc
    u8 m_dd;                    // 0xdd
    u8 m_de;                    // 0xde
    u8 mPadDF;                  // 0xdf
    int mGroupNo;               // 0xe0

    static dBg_ctr_c *mEntryN;
    static dBg_ctr_c *mEntryB;
    static dActor_c *mGroupCtrlActor;
    static int mGroupCtrlNo;

    dBg_ctr_c();
    ~dBg_ctr_c();

    void reset();
    void init();
    void entry();
    void release();
    void set_common(dActor_c *, CallbackF *, CallbackH *, CallbackW *, u8, u8);
    void set(dActor_c *, float, float, float, float, CallbackF *, CallbackH *,
             CallbackW *, u8, u8, mVec3_c *);
    void set(dActor_c *, mVec2_c, mVec2_c, CallbackF *, CallbackH *, CallbackW *,
             u8, u8, mVec3_c *);
    void set(dActor_c *, const sBgSetInfo *, u8, u8, mVec3_c *);
    void set_circle(dActor_c *, float, float, float, CallbackF *, CallbackH *,
                    CallbackW *, u8, u8);
    void setOfs(float, float, float, float, mVec3_c *);
    void setOfs(mVec2_c, mVec2_c, mVec3_c *);
    void setOfsX1(float);
    void setOfsY1(float);
    void setOfsX2(float);
    void setOfsY2(float);
    void setAngleY3(short *);
    void calc();
    void revisePos();
    void addDokanMoveDiff(mVec3_c *);
    bool fn_80080E40(dBc_c *, u8, u8);
    bool fn_80080880(f32, f32, f32);
    bool fn_80080900(mVec3_c *, short *, int);
    bool fn_80080670(mVec3_c *, f32);
    bool setLinkNetPlayer(dBc_c *);
    dBc_c *getLinkNetPlayer(signed char);
    bool setLinkWallSlidPlayer(dBc_c *);
    void update();
    void updateObjBg();
    bool upperRevCheck(dActor_c *);
    bool underRevCheck(dActor_c *);
    bool sideRevCheck(dActor_c *, u8);
    static bool CheckRevUpperSpeed(dActor_c *, dActor_c *);
    static bool CheckRevUnderSpeed(dActor_c *, dActor_c *);
    static bool CheckRevSideSpeed(dActor_c *, dActor_c *, u8);
    int checkRevisionState(ulong);
};
