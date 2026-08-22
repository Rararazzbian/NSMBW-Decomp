#pragma once
#include <game/mLib/m_vec.hpp>

class daBossDemo_c;

class dActorMng_c {
public:
    u8 mPad1[0x18];
    daBossDemo_c *mpBossDemo;
    u8 mPad1b[0xC];
    int mGoombaZOrderThing;
    u8 mPad2a[0x18];
    float mGoalPoleX;
    u8 mPad2[0x1fc];

    void createUpCoin(const mVec3_c &pos, u8 dir, u8 count, u8 layer);
    void createJumpCoin(const mVec3_c &pos, u8 count, u8 layer);
    void createBlockDownCoin(const mVec3_c &pos, u8 count, u8 layer);

    bool envObakeCheck();
    bool envAllWaterCheck();
    bool floorEntryBufferCheck(mVec2_c *pt);

    static dActorMng_c *m_instance;
};
