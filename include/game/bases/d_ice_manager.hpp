#pragma once

#include <game/bases/d_actor.hpp>

class dIceEfScale_c {
public:
    dIceEfScale_c() {
        mData[0] = 0.0f;
        mData[1] = 0.0f;
        mData[2] = 0.0f;
        mData[3] = 0.0f;
        mData[4] = 0.0f;
        mData[5] = 0.0f;
        mData[6] = 0.0f;
        mData[7] = 0.0f;
    }

    dIceEfScale_c(float s0, float s1, float s2, float s3, float s4, float s5, float s6, float s7) {
        mData[0] = s0;
        mData[1] = s1;
        mData[2] = s2;
        mData[3] = s3;
        mData[4] = s4;
        mData[5] = s5;
        mData[6] = s6;
        mData[7] = s7;
    }

    float mData[8];
};

class dIceInfo {
public:
    ~dIceInfo();

    int mMode;
    mVec3_c mPos;
    mVec3_c mSize;
    dIceEfScale_c mEfScale;
};

class dIceMng_c {
public:
    /// @unofficial
    enum PROC_e {
        PROC_FROZEN,
        PROC_MELT,
        PROC_2,
        PROC_DEFAULT
    };

    /// @unofficial
    enum DESTROY_MODE_e {
        DESTROY_NONE,
        DESTROY_BREAK,
        DESTROY_VANISH = 3
    };

    dIceMng_c(dActor_c *owner);
    ~dIceMng_c();

    void initialize();
    void setIceStatus(int, int, int); ///< @unofficial
    PROC_e manageProc();
    void breakEffect();
    void removeIce();
    bool checkInstantBreak(int);
    bool createIce(dIceInfo *info, int count);

    u8 mPad1[0xc];
    int mActive;
    u8 mPad2[0x8];
    DESTROY_MODE_e mDestroyMode;
    u8 mPad3[0x14];
    int mPlrNo;
    u8 mPad4[0x38];
};
