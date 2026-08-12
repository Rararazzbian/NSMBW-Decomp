#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_effect.hpp>

/**
 * @brief Base class for Goombas ("kuribo").
 * @statetable
 * @paramtable
 */
class daEnKuriboBase_c : public dEn_c {
public:
    virtual ~daEnKuriboBase_c();

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void finalUpdate();

    virtual void Normal_VsEnHitCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other);

    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, DieOther);

    virtual void fumidamageSE(const mVec3_c &pos, int playerNo);
    virtual bool setDamage(dActor_c *actor);
    virtual void beginFunsui();
    virtual void endFunsui();
    virtual BOOL isFunsui() const;

    // New virtual functions

    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, Walk); ///< Walking on the ground.
    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, Turn); ///< Turning around.
    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, TrplnJump); ///< Bouncing on a trampoline.

    virtual void calcModel();
    virtual void reactFumiProc(dActor_c *actor);
    virtual void reactMameFumiProc(dActor_c *actor);
    virtual void reactSpinFumiProc(dActor_c *actor);
    virtual void reactYoshiFumiProc(dActor_c *actor);
    virtual void createModel();
    virtual void createBodyModel();
    virtual void createOtherModel();
    virtual void drawModel();
    virtual void initialize();
    virtual void setTurnByEnemyHit();
    virtual void setTurnByPlayerHit(dActor_c *actor);
    virtual void setWalkSpeed();
    virtual void setWalkAnm();
    virtual void playWalkAnm();
    virtual void walkEffect();
    virtual bool isWakidashi() const;
    virtual bool isDamageInvalid();
    virtual BOOL isBgmSync() const;

    // Nonvirtuals

    bool checkRyusa(); ///< Checks if the Goomba is standing in quicksand.
    void ryusaEffect(); ///< Creates the quicksand effect.
    void setLayerPos(); ///< Updates the Z position based on the chainlink fence layer.
    void setDeathInfo_Hasami(); ///< Kills the Goomba after being crushed.
    void setCcLine(); ///< Updates the collider's chainlink fence layer.
    void landonEffect(); ///< Creates the landing dust effect.
    bool isOnTrampoline(); ///< @unofficial

    dHeapAllocator_c mAllocator; ///< The allocator used for the resources of this actor.
    nw4r::g3d::ResFile mResFile; ///< The resource file for the Goomba.
    m3d::mdl_c mModel; ///< The Goomba model.
    m3d::anmChr_c mAnmChr; ///< The Goomba's animation.
    nw4r::g3d::ResAnmTexPat mResAnmTexPat; ///< The animated texture resource of the Goomba.
    m3d::anmTexPat_c mAnmTexPat; ///< The animated texture of the Goomba.

    float mBaseZPos; ///< The base Z position of the Goomba.
    BOOL mIsFunsui; ///< Whether the Goomba is being blown upwards by a fountain.
    float mXSpeedBeforeFunsui; ///< The horizontal speed before being blown by a fountain.
    mEf::levelEffect_c mEffect;
    mEf::levelOneEffect_c mQuicksandEffect;
    u16 mScrOutFlags; ///< The flags passed to dActor_c::ActorScrOutCheck.

    ACTOR_PARAM_CONFIG(SubLayer, 16, 1); ///< The sublayer to spawn on.

    static const s16 smc_TURN_SPEED;
    static const float smc_MAX_XSPEED;
    static const float smc_MAX_YSPEED;
};
