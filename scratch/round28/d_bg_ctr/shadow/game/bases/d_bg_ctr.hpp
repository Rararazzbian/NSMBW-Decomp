#pragma once

class dBg_ctr_c;

#include <game/bases/d_actor.hpp>
#include <game/bases/d_bc.hpp>
#include <game/mLib/m_vec.hpp>

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

    dActor_c *mpActor;
    dBg_ctr_c *mEntryPrev;
    dBg_ctr_c *mEntryNext;
    int m_0c;
    int m_10;
    int m_14;
    dBc_c *mLinkNetPlayer[4];
    dBc_c *mWallSlidPlayer[4];
    int m_38;
    int mUpdateFlag;
    CallbackF *mCallbackF;
    CallbackH *mCallbackH;
    CallbackW *mCallbackW;
    CheckRevF *mCheckRevUpper;
    CheckRevF *mCheckRevUnder;
    CheckRevW *mCheckRevSide;
    mVec2_c mPos;
    mVec2_c mScratch[4];
    mVec2_c mCenter;
    mVec2_c mOffset2;
    f32 mRadius;
    u8 mPad94[0xc];
    mVec2_c m_a0;
    mVec2_c m_a8;
    mVec2_c m_ac;
    u8 mPadB8[4];
    short *mRotation;
    short m_c0;
    short m_c2;
    short m_c4;
    u8 mPadC6[0x2];
    int mMode;
    u32 mFlags2;
    int mFlags;
    int m_d4;
    int m_d8;
    bool mEntryFlag;
    u8 m_dd;
    u8 m_de;
    u8 mPadDF;
    int mGroupNo;

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
    void set(dActor_c *, float, float, float, float, CallbackF *, CallbackH *, CallbackW *, u8, u8, mVec3_c *);
    void set(dActor_c *, mVec2_c, mVec2_c, CallbackF *, CallbackH *, CallbackW *, u8, u8, mVec3_c *);
    void set(dActor_c *, const sBgSetInfo *, u8, u8, mVec3_c *);
    void set_circle(dActor_c *, float, float, float, CallbackF *, CallbackH *, CallbackW *, u8, u8);
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
