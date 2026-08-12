#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>

/**
 * @brief Base implementation of a piranha plant ("pakkun") enemy.
 * @details The plant hides inside a pipe, rises out of it, snaps at the players
 * and sinks back in. Only the shared behaviour lives here; the state logic is
 * supplied by the subclasses (@ref daEnDpakkun_c and @p daEnDfpakkun_c).
 * @unofficial
 */
class daEnDpakkunBase_c : public dEn_c {
public:
    /// @brief Bone callback used to bend the plant's neck. @unofficial
    class nodeCallback_c : public m3d::mdl_c::callback_c {
    public:
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl);

        daEnDpakkunBase_c *mpOwner; ///< 0x04 @unofficial
    };

    /// @brief Constructs the enemy.
    /// @details Binds the bone callback to its owner; @ref nodeCallback_c::timingA
    /// dereferences @p mpOwner and nothing else ever assigns it.
    daEnDpakkunBase_c() { mNodeCallback.mpOwner = this; }

    /// @brief Destroys the enemy.
    /// @details Defined inline: the surviving out-of-line copy is linked from
    /// @p d_a_en_dfpakkun.cpp, not from @p d_a_en_dpakkun_base.cpp.
    virtual ~daEnDpakkunBase_c() {}

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual int draw();
    virtual int preDraw();
    virtual void deleteReady();

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual void removeCc() {
        dActor_c::mCc.release();
        mCc.release();
    }

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual void reviveCc() {
        dActor_c::mCc.entry();
        mCc.entry();
    }

    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Star(dCc_c *self, dCc_c *other);

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual bool hitCallback_Spin(dCc_c *self, dCc_c *other) { return false; }

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual bool hitCallback_HipAttk(dCc_c *self, dCc_c *other) { return false; }

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual bool hitCallback_Slip(dCc_c *self, dCc_c *other) { return false; }

    virtual bool hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other);

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual bool hitCallback_PenguinSlide(dCc_c *self, dCc_c *other) {
        setDeathInfo_Smoke(nullptr);
        return true;
    }

    virtual bool hitCallback_Shell(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_YoshiFire(dCc_c *self, dCc_c *other);

    virtual void setDeathInfo_Quake(int type);
    virtual void setDeathInfo_IceBreak();
    virtual BOOL isQuakeDamage();
    virtual void setIceAnm();
    virtual void YoshiFumiJumpSet(dActor_c *actor);

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual void YoshiFumiScoreSet(dActor_c *actor) {}

    // New virtual functions (vtable 0x280..0x2DC)

    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, Wait);        ///< Hiding inside the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, Appear);      ///< Rising out of the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, Attack);      ///< Snapping at the players.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, Disappear);   ///< Sinking back into the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, DieIceBreak); ///< Shattering after being frozen.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, DieVanish);   ///< Withering away.

    virtual void setIceBreakAnm();
    virtual void initialize();

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual void initPakkunDir() {}

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    /// @note Declared here, in the middle of the new virtuals, rather than up
    /// with the other base overrides. It is an *override*, so its vtable slot
    /// comes from @ref dEn_c and is unaffected by where it appears -- and
    /// uncalled inline members are flushed at the end of the TU in reverse
    /// declaration order, so this position is what puts it where the original
    /// has it. Same reason @ref hitCallback_Spin, @ref hitCallback_HipAttk and
    /// @ref hitCallback_Slip are declared in that order above.
    virtual void finalUpdate() { calcMdl(); }

    virtual void createMdl();
    virtual void setVanishAnm();

    /// @details Defined inline; the linked copy comes from @p d_a_en_dfpakkun.cpp.
    virtual void updateCc() {}

    // Nonvirtuals

    void allocate();
    void calcMdl();
    void calcJnt();
    bool isPlayerDemo();
    void kill(dActor_c *killedBy);
    bool checkQuakeDeath();
    void setDeathInfo_Vanish(dActor_c *killedBy);

    dHeapAllocator_c mAllocator;   ///< 0x524 @unofficial
    nw4r::g3d::ResFile mResFile;   ///< 0x540 @unofficial
    m3d::mdl_c mModel;             ///< 0x544 @unofficial
    m3d::anmChr_c mAnm;            ///< 0x584 @unofficial
    u8 m_5bc[0x5c4 - 0x5bc];       ///< 0x5BC @unused
    mVec3_c mNodePos7;             ///< 0x5C4 World position of bone 7. @unofficial
    mVec3_c mNodePos6;             ///< 0x5D0 World position of bone 6. @unofficial
    mVec3_c mFirePos;              ///< 0x5DC #mMouthPos, moved 48 units towards the camera. @unofficial
    mVec3_c mNodePos5;             ///< 0x5E8 World position of bone 5. @unofficial
    mVec3_c mMouthPos;             ///< 0x5F4 Bone 6, offset by 6 units along its own Y axis. @unofficial
    dCc_c mCc;                     ///< 0x600 The head collider. @unofficial
    int mTimer;                    ///< 0x6A4 State frame counter. @unofficial
    int mPakkunDir;                ///< 0x6A8 Direction the plant grows in (0-3). @unofficial
    int mIsDying;                  ///< 0x6AC Whether the plant has already been killed. @unofficial
    int mAttacking;                ///< 0x6B0 @unofficial
    s16 mNeckAngle[9];             ///< 0x6B4 Extra Z rotation applied to each neck bone. @unofficial
    mVec3_c mStartPos;             ///< 0x6C8 Position the plant was spawned at. @unofficial
    s16 mSpinAngle;                ///< 0x6D4 Z rotation applied around the pivot point. @unofficial
    nodeCallback_c mNodeCallback;  ///< 0x6D8 @unofficial

    static const sCcDatNewF smc_cc_dat; ///< @unofficial

    ACTOR_PARAM_CONFIG(NoFireDamage, 16, 1); ///< @unofficial
};

extern const mVec2_c l_hole_offset[4]; ///< The pipe mouth offset for each growth direction. @unofficial
