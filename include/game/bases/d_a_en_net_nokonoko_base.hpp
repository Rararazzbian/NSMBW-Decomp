#pragma once
#include <game/bases/d_a_net_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>

/**
 * @brief Base implementation of a Koopa Troopa walking on a chainlink fence.
 * @statetable
 * @paramtable
 */
class daEnNetNoko_c : public daNetEnemy_c {
public:
    virtual ~daEnNetNoko_c();

    virtual int create();
    virtual int draw();
    virtual int doDelete();
    virtual void deleteReady();

    virtual bool setEatSpitOut(dActor_c *eatingActor);

    virtual bool PlDamageCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other);

    virtual BOOL isQuakeDamage();

    STATE_VIRTUAL_FUNC_DECLARE(daEnNetNoko_c, DieFumi);
    STATE_VIRTUAL_FUNC_DECLARE(daEnNetNoko_c, DieFall);

    virtual void MameFumiJumpSet(dActor_c *actor);

    virtual void mdlPlay();
    virtual void calcMdl();

    // New virtual functions

    virtual void initialize();
    virtual void entryHIO();
    virtual void removeHIO();

    // Nonvirtuals

    void createMdl();
    void setColor();
    int wireBgCheck(const mVec2_c &offset);

    dHeapAllocator_c mAllocator; ///< The allocator used for the resources of this actor.
    nw4r::g3d::ResFile mResFile; ///< The resource file containing the resources of this actor.
    m3d::mdl_c mModel; ///< The model of the Koopa.
    m3d::anmChr_c mAnim; ///< The current animation of the Koopa.
    nw4r::g3d::ResAnmTexPat mResAnmTexPat; ///< The animated texture resource of the Koopa.
    m3d::anmTexPat_c mAnimTex; ///< The animated texture of the Koopa.
    int mColor; ///< The Koopa's colour (green or red).

    ACTOR_PARAM_CONFIG(Color, 0, 1);
    ACTOR_PARAM_CONFIG(AmiLayer, 4, 1);

    static const sBcSensorPoint smc_noko_head;
    static const sBcSensorPoint smc_noko_foot;
    static const sBcSensorPoint smc_noko_wall;

    static const float smc_MOVE_SPEED[2];
    static const float smc_MOVE_SPEED_HIGH[2];
    static const s16 smc_ANGLE_Y[2];
};
