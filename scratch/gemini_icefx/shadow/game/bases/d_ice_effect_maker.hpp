#pragma once
#include <game/bases/d_base_actor.hpp>
#include <game/bases/d_ice_manager.hpp>

class dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos) = 0;
    virtual int follow(const mVec3_c &pos) = 0;

    mVec3_c mScale;
};

class dIceFreezeEf_c : public dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos);
    virtual int follow(const mVec3_c &pos);

    u8 mPad[0x124 - sizeof(dIceEfInf_c)];
};

class dIceSmokeEf_c : public dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos);
    virtual int follow(const mVec3_c &pos);

    u8 mPad[0x138 - sizeof(dIceEfInf_c)];
};

class dIceBreakEf_c : public dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos);
    virtual int follow(const mVec3_c &pos);

    u8 mPad[0x124 - sizeof(dIceEfInf_c)];
};

class dIceReleaseEf_c : public dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos);
    virtual int follow(const mVec3_c &pos);

    u8 mPad[0x124 - sizeof(dIceEfInf_c)];
};

class dIceThawEf_c : public dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos);
    virtual int follow(const mVec3_c &pos);

    u8 mPad[0x124 - sizeof(dIceEfInf_c)];
};

class dIceYoganEf_c : public dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos);
    virtual int follow(const mVec3_c &pos);

    u8 mPad[0x124 - sizeof(dIceEfInf_c)];
};

class dIcePoisonEf_c : public dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos);
    virtual int follow(const mVec3_c &pos);

    u8 mPad[0x124 - sizeof(dIceEfInf_c)];
};

class dIceWaterBreakEf_c : public dIceEfInf_c {
public:
    virtual void create(const mVec3_c &pos);
    virtual int follow(const mVec3_c &pos);

    u8 mPad[0x10 - sizeof(dIceEfInf_c)];
};

class dIceEfMaker_c {
public:
    enum EfKind_e {
        FREEZE,
        SMOKE,
        BREAK,
        RELEASE,
        THAW,
        YOGAN,
        POISON,
        WATER_BREAK,
        NUM_EFFECTS = 8
    };

    void init(int kind, dIceEfScale_c *scale);
    void execute();
    void fin();
    void setEfScale(const dIceEfScale_c &scale);
    void createEffect(EfKind_e kind);
    void hahenEffect();

    u32 mActiveFlags;                 // +0x000
    int mMode;                        // +0x004
    dIceFreezeEf_c mFreezeEf;         // +0x008
    dIceSmokeEf_c mSmokeEf;           // +0x12C
    dIceBreakEf_c mBreakEf;           // +0x264
    dIceReleaseEf_c mReleaseEf;       // +0x388
    dIceThawEf_c mThawEf;             // +0x4AC
    dIceYoganEf_c mYoganEf;           // +0x5D0
    dIcePoisonEf_c mPoisonEf;         // +0x6F4
    dIceWaterBreakEf_c mWaterBreakEf; // +0x818
    dIceEfInf_c *mpEffects[8];        // +0x828
    dBaseActor_c *mpActor;            // +0x848
};
