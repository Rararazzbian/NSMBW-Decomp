#pragma once

#include <game/mLib/m_vec.hpp>

// SHADOW COPY of include/game/bases/d_actor_manager.hpp, rebased on the revision
// that added mGoalPoleX at +0x44. The ONLY delta is envAllWaterCheck(), which
// d_a_en_hatena_balloon.cpp's model_set() calls twice and which the real header
// does not yet declare. Landing this unit needs that one line added upstream;
// it is layout-neutral (non-virtual, no data members touched).

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

    static dActorMng_c *m_instance;
};
