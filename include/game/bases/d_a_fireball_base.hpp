#pragma once

#include <game/bases/d_actor_state.hpp>
#include <game/bases/d_circle_light_mask.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_effect.hpp>

/// @brief Base implementation of a fireball-like projectile.
/// @ingroup bases
class daFireBall_Base_c : public dActorState_c {
public:
    daFireBall_Base_c() : mIsDead(0), mLiquidType(0), m_414(1), m_428(0) {}
    virtual ~daFireBall_Base_c() {}

    // dActor_c overrides
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int preExecute();
    virtual int draw();
    virtual void deleteReady();
    virtual void setEatTongue(dActor_c *eatingActor);
    virtual void setEatTongueOff(dActor_c *eatingActor);

    STATE_VIRTUAL_FUNC_DECLARE(daFireBall_Base_c, Move);
    STATE_VIRTUAL_FUNC_DECLARE(daFireBall_Base_c, Kill);
    STATE_VIRTUAL_FUNC_DECLARE(daFireBall_Base_c, EatIn);
    STATE_VIRTUAL_FUNC_DECLARE(daFireBall_Base_c, EatNow);

    // New virtual functions
    virtual int initialize();
    virtual int createCheck();
    virtual void setCc();
    virtual void setBc();
    virtual void chgZpos();
    virtual void fireEffect();
    virtual void beginSplash(float height);
    virtual void beginYoganSplash(float height);
    virtual void beginPoisonSplash(float height);
    virtual float getLightRad() const;
    virtual void entryHIOnode();
    virtual void retireHIOnode();

    // Nonvirtuals
    bool cullCheck();
    void kill();
    void lightProc();

    dHeapAllocator_c mAllocator;   ///< 0x3D0
    dCircleLightMask_c mLightMask; ///< 0x3EC
    u32 mIsDead;                   ///< 0x408 Set by kill(), checked by preExecute().
    int mKillTimer;                ///< 0x40C Set to 10 by initializeState_Kill().
    int mLiquidType;               ///< 0x410 dBc_c::WATER_TYPE_e
    u32 m_414;                     ///< 0x414 Initialized to 1.
    float mLiquidHeight;           ///< 0x418
    mVec3_c mStartPos;             ///< 0x41C
    u32 m_428;                     ///< 0x428
    mEf::levelEffect_c mEffect;    ///< 0x42C

    static const float smc_MAXFALLSPEED;
    static const float smc_GRAVITY;
};
