#pragma once

#include <game/bases/d_a_fireball_base.hpp>

/// @brief A fireball, thrown by the player.
/// @paramtable
/// @statetable
/// @ingroup bases
class daFireBall_Player_c : public daFireBall_Base_c {
public:
    daFireBall_Player_c() {} ///< Creates a new player fireball actor.
    virtual ~daFireBall_Player_c() {} ///< Destroys the player fireball actor.

    virtual int doDelete();
    virtual int initialize();
    virtual int createCheck();
    virtual void setCc();
    virtual void chgZpos();
    virtual void beginSplash(float height);
    virtual void entryHIOnode();
    virtual void retireHIOnode();

    STATE_VIRTUAL_FUNC_DECLARE(daFireBall_Player_c, Move); ///< The fireball is bouncing along the ground.

    /// @brief Checks whether the fireball should vanish on creation.
    bool checkInitVanish();

    /// @brief Checks if the fireball is close to the ground so that it can spawn a bit higher up.
    bool checkInitLine(float &groundHeight);

    bool killcheck_Bg(); ///< Checks if the fireball should be destroyed due to the tile it landed on.
    bool killcheck_Ride(); ///< Checks if the fireball should be destroyed due to the platform it landed on.
    u32 boundCheck(); ///< Checks if the fireball touched the ground.

    /// @brief Sets the bounce speed for the slope the fireball landed on.
    void setSakaSpeed(u8 sakaType, u8 sakaDir);

    static void fireball_collcheck(dCc_c *self, dCc_c *other); ///< Collision callback for the fireball.

    /// @brief Checks if a new fireball can be created for a player based on the limit mode.
    static bool CheckFireBallLimit(int playerNo, int limitMode);

    ACTOR_PARAM_CONFIG(PlayerNo, 0, 2); ///< The player who threw the fireball.
    ACTOR_PARAM_CONFIG(Direction, 4, 1); ///< The direction the fireball was thrown.
    ACTOR_PARAM_CONFIG(Layer, 8, 2); ///< The layer the fireball is on.
    ACTOR_PARAM_CONFIG(AmiLine, 12, 2); ///< The chainlink fence layer for the fireball.
    ACTOR_PARAM_CONFIG(LimitMode, 16, 2); ///< The limit mode for the fireball.

    int mPlayerNum; ///< 0x554 The player who threw the fireball.
    int mAliveTimer; ///< 0x558 Timer for the fireball's lifetime.

    static int sm_FireBallCnt[4]; ///< The number of fireballs that currently exist for each player.
    static int sm_AliveFireBallCnt[4]; ///< The number of "alive" fireballs for each player.

    static const int smc_MAX_FIREBALL_COUNT = 6; ///< @unofficial
    static const int smc_MAX_ALIVE_FIREBALL_COUNT = 2; ///< @unofficial
};
