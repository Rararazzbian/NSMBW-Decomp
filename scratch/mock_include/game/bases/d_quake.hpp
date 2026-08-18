#pragma once

#include <types.h>
#include <game/mLib/m_vec.hpp>

class dQuake_c {
public:
    enum TYPE_SHOCK_e {
        TYPE_0,
        TYPE_1,
        TYPE_2,
        TYPE_3,
        TYPE_4,
        TYPE_5,
        TYPE_6,
        TYPE_7,
        TYPE_8
    };

    enum FLAGS_e {
        FLAG_0 = BIT_FLAG(0), ///< 0x01: Screen/enemy ground shake (dEn_c::quakeAction).
        FLAG_1 = BIT_FLAG(1), ///< 0x02: POW shock kill (set while mPOWLength > 0).
        FLAG_2 = BIT_FLAG(2), ///< 0x04: Mega Ground Pound / Super POW shock kill (set while mMPGPLength > 0).
        FLAG_3 = BIT_FLAG(3), ///< 0x08: Player small quake hop (unconditional, triggers STATUS_QUAKE_SMALL 0x8C). @unofficial
        FLAG_4 = BIT_FLAG(4), ///< 0x10: Player small quake hop (triggers STATUS_QUAKE_SMALL 0x8C if m_quakeTimer == 0). @unofficial
        FLAG_5 = BIT_FLAG(5), ///< 0x20: Player big quake stun (triggers STATUS_QUAKE_BIG 0x8B). @unofficial
    };

    void shockMotor(s8, TYPE_SHOCK_e, int, bool);
    void startShockAll(TYPE_SHOCK_e, int, int, bool);

    float mSpeed;
    float mResult;
    float mMultiplier;
    float mAccel;
    float mMin;
    float mSpeed2;
    float mResult2;
    float mMultiplier2;
    float mAccel2;
    float mMin2;
    mVec2_c mScreenOffset;
    FLAGS_e mFlags;
    u32 m_34;
    bool mQuakeOn;
    u8 mPlayerId;
    u8 mPadding[2];
    int mPOWLength;
    int mMPGPLength;

    static dQuake_c* getInstance() { return m_instance; }

    static dQuake_c *m_instance;
};
