#pragma once

#include <game/mLib/m_vec.hpp>

class dActorMng_c {
public:
    u8 mPad1[0x28];
    int mGoombaZOrderThing;
    u8 mPad2a[0x18];
    /// @brief [0x44] X of the level's goal pole. @unofficial
    float mGoalPoleX;
    u8 mPad2[0x1fc];

    void createUpCoin(const mVec3_c &pos, u8 dir, u8 count, u8 layer);
    void createJumpCoin(const mVec3_c &pos, u8 count, u8 layer);
    void createBlockDownCoin(const mVec3_c &pos, u8 count, u8 layer);

    /// @brief Whether the level is in its "ghost house" lighting mode. @unofficial
    bool envObakeCheck();

    /// @brief Whether the whole level is underwater. @unofficial
    bool envAllWaterCheck();

    /// @brief Whether @p pt lies inside the level's floor-entry buffer. @unofficial
    bool floorEntryBufferCheck(mVec2_c *pt);

    static dActorMng_c *m_instance;
};
