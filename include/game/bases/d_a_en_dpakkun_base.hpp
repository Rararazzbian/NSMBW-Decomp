#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>

/**
 * @brief Base implementation of a piranha plant ("pakkun") enemy.
 * @details Reconstructed from @p __vt__17daEnDpakkunBase_c and the object layout
 * used by @ref daEnDpakkun_c and @p daEnDfpakkun_c. The translation unit itself
 * (@p d_a_en_dpakkun_base.cpp, .text 0x8002CB70-0x8002EBA4) is **not** decompiled
 * yet, so member names below marked @unofficial are inferred.
 * @unofficial
 */
class daEnDpakkunBase_c : public dEn_c {
public:
    /// @brief Bone callback used to bend the plant's neck. @unofficial
    class nodeCallback_c : public m3d::mdl_c::callback_c {
    public:
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl);

        daEnDpakkunBase_c *mpOwner; ///< @unofficial
    };

    /// @brief Destroys the enemy.
    /// @details Defined inline: both known subclasses inline the member
    /// destruction sequence into their own destructors rather than calling
    /// @p __dt__17daEnDpakkunBase_cFv.
    virtual ~daEnDpakkunBase_c() {}

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual int draw();
    virtual int preDraw();
    virtual void deleteReady();
    virtual void finalUpdate();

    virtual void removeCc();
    virtual void reviveCc();

    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Star(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Slip(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Spin(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_HipAttk(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_PenguinSlide(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Shell(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_YoshiFire(dCc_c *self, dCc_c *other);

    virtual void setDeathInfo_Quake(int);
    virtual void setDeathInfo_IceBreak();
    virtual BOOL isQuakeDamage();
    virtual void setIceAnm();
    virtual void YoshiFumiJumpSet(dActor_c *actor);
    virtual void YoshiFumiScoreSet(dActor_c *actor);

    // New virtual functions (vtable 0x280..0x2DC)

    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, Wait);        ///< Hiding inside the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, Appear);      ///< Rising out of the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, Attack);      ///< Snapping at the players.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, Disappear);   ///< Sinking back into the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, DieIceBreak); ///< Shattering after being frozen.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkunBase_c, DieVanish);   ///< Withering away.

    virtual void setIceBreakAnm();
    virtual void initialize();
    virtual void initPakkunDir();
    virtual void createMdl();
    virtual void setVanishAnm();
    virtual void updateCc();

    // Nonvirtuals

    void allocate();
    void calcMdl();
    void calcJnt();
    bool isPlayerDemo();
    void kill(dActor_c *killedBy);
    bool checkQuakeDeath();
    void setDeathInfo_Vanish(dActor_c *killedBy);

    dHeapAllocator_c mAllocator;      ///< 0x524 @unofficial
    nw4r::g3d::ResFile mResFile;      ///< 0x540 @unofficial
    m3d::mdl_c mModel;                ///< 0x544 @unofficial
    m3d::anmChr_c mAnm;               ///< 0x584 @unofficial
    u8 m_5bc[0x600 - 0x5bc];          ///< 0x5BC @unused
    dCc_c mCc;                        ///< 0x600 @unofficial
    int mTimer;                       ///< 0x6A4 State frame counter. @unofficial
    int mPakkunDir;                   ///< 0x6A8 Direction the plant grows in (0-3). @unofficial
    dActor_c *mpKiller;               ///< 0x6AC @unofficial
    int mAttacking;                   ///< 0x6B0 @unofficial
    s16 m_6b4[9];                     ///< 0x6B4 @unofficial
    mVec3_c mStartPos;                ///< 0x6C8 Position the plant was spawned at. @unofficial
    s16 m_6d4;                        ///< 0x6D4 @unofficial
    nodeCallback_c mNodeCallback;     ///< 0x6D8 @unofficial
};
