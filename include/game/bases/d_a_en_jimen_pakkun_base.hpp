#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_mat_clr.hpp>

/**
 * @brief Base implementation of a ground-dwelling piranha plant ("jimen pakkun").
 * @details The plant grows straight out of the ground (or, with the @p Sakasa
 * parameter set, out of a ceiling upside-down), snaps at the players and can be
 * frozen, burned or crushed. The attack behaviour itself is left to the
 * subclasses, which supply the @ref StateID_Attack and @ref StateID_SakasaAttack
 * bodies. Its four known subclasses all live in @p d_enemiesNP.rel:
 * @p EN_JIMEN_PAKKUN, @p EN_JIMEN_FPAKKUN, @p EN_JIMEN_BIG_PAKKUN and
 * @p EN_JIMEN_BIG_FPAKKUN.
 *
 * @note Despite the name, this is @em not a @p daEnDpakkunBase_c. It derives
 * straight from @ref dEn_c: its vtable is byte-for-byte @p dEn_c's for all 160
 * inherited slots (0x008..0x27C, @p __vt__5dEn_c is exactly 0x280 bytes) and
 * only then adds its own. Compare @ref daEnLkuriboBase_c, which is the same
 * kind of name/parent mismatch.
 * @statetable
 * @unofficial
 */
class daEnJimenPakkunBase_c : public dEn_c {
public:
    /// @brief Destroys the enemy.
    /// @details Defined inline. It is emitted last in the end-of-TU inline
    /// flush block (0x80030A00), i.e. it is the first of that block's members
    /// to be declared -- the block comes out in reverse declaration order.
    /// Same shape as @ref daEnDpakkunBase_c.
    virtual ~daEnJimenPakkunBase_c() {}

    // Base class overrides
    // An override inherits its slot from dEn_c, so the order in this section is
    // free as far as the vtable is concerned. What it *does* fix is the order of
    // the end-of-TU inline flush block; see the notes on the inline members.

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual int draw();
    virtual void deleteReady();

    /// @details Defined inline. It is called from @ref initializeState_DieOther
    /// (and again from @ref initializeState_DieIceBreak) and lands at
    /// 0x80030500, immediately after that first caller -- the lazy-flush
    /// position, and the same shape @ref daEnDpakkunBase_c uses. The member
    /// order below is the order @p create, @p doDelete and @p reviveCc use too.
    virtual void removeCc() {
        dActor_c::mCc.release();
        mHeadCc.release();
        mBodyCc.release();
    }

    /// @details Out of line: nothing in this TU calls it, and it sits at
    /// 0x800308D0, a natural source position between @ref executeState_SakasaAttack
    /// and @ref finalUpdate -- not in the end-of-TU flush block.
    virtual void reviveCc();

    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Star(dCc_c *self, dCc_c *other);

    /// @details Inline; part of the end-of-TU flush block (0x800309F0).
    virtual bool hitCallback_Spin(dCc_c *self, dCc_c *other) { return false; }

    /// @details Inline; part of the end-of-TU flush block (0x800309E0).
    virtual bool hitCallback_HipAttk(dCc_c *self, dCc_c *other) { return false; }

    /// @details Inline; part of the end-of-TU flush block (0x800309D0).
    /// @note Spin, HipAttk and Slip must be declared in exactly this order --
    /// the block emits in reverse declaration order. Identical to the ordering
    /// @ref daEnDpakkunBase_c needed.
    virtual bool hitCallback_Slip(dCc_c *self, dCc_c *other) { return false; }

    virtual bool hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Shell(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Ice(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_YoshiFire(dCc_c *self, dCc_c *other);

    virtual void setDeathInfo_Quake(int type);
    virtual void setDeathInfo_IceBreak();
    virtual BOOL isQuakeDamage();

    virtual bool createIceActor();
    virtual void setIceAnm();
    virtual void returnState_Ice();

    virtual bool ActorDrawCullCheck();
    virtual void finalUpdate();
    virtual void YoshiFumiJumpSet(dActor_c *actor);

    /// @details Inline; part of the end-of-TU flush block (0x800309C0).
    virtual void block_hit_init() {}

    /// @details Inline; emitted first in the end-of-TU flush block (0x800309B0),
    /// so it is the last of that block's members to be declared.
    virtual void YoshiFumiScoreSet(dActor_c *actor) {}

    /// @brief Withering away after being killed by anything without its own
    /// death animation.
    /// @note An override of @ref dEn_c's state of the same name, so it takes
    /// dEn_c's three vtable slots (0x1C0..0x1C8) and declares its own
    /// @p StateID_DieOther, which shadows dEn_c's. Any reference to the base
    /// state from code in this class must therefore be written
    /// @p &dEn_c::StateID_DieOther.
    STATE_VIRTUAL_FUNC_DECLARE(daEnJimenPakkunBase_c, DieOther);

    // New virtual functions (vtable 0x280..0x2B8).
    // The order of THIS section is fixed by the vtable -- do not reorder.

    STATE_VIRTUAL_FUNC_DECLARE(daEnJimenPakkunBase_c, DieIceBreak);  ///< 0x280 Shattering after being frozen.
    STATE_VIRTUAL_FUNC_DECLARE(daEnJimenPakkunBase_c, Attack);       ///< 0x28C Snapping at the players. Empty here.
    STATE_VIRTUAL_FUNC_DECLARE(daEnJimenPakkunBase_c, SakasaAttack); ///< 0x298 Snapping while upside-down. Empty here.

    /// @brief The Y offset applied to the plant when it grows downwards. @unofficial
    /// @details 0x2A4. Defined inline: it is called from @ref initAction and
    /// lands at 0x8002F6E0, immediately after that caller.
    virtual float getSakasaOfs() const { return 33.0f; }

    virtual void initialize();  ///< 0x2A8
    /// @note Returns int, not void -- the body ends `li r3, 0x1; blr`. MWCC
    /// does not mangle return types, so this changes no symbol and moves no
    /// vtable slot; it is invisible to a vtable comparison and has to come from
    /// the body.
    virtual int initAction();  ///< 0x2AC @unofficial

    /// @brief Plays the "plant sinks back down" sound effect (SE id 0x1A8). @unofficial
    /// @details 0x2B0. Its body MUST be emitted at 0x80030540, immediately after
    /// @ref removeCc: @ref initializeState_DieOther calls removeCc one
    /// instruction before it, so the two are flushed in that order. Declared
    /// here rather than defined inline only to keep @p d_audio.hpp out of this
    /// header -- define it out of line immediately after
    /// @ref initializeState_DieOther in the .cpp, or give it an inline body
    /// here; both reach that address.
    virtual void downSE();

    /// @details 0x2B4. Defined inline: called as the last thing @ref create does
    /// and lands at 0x8002F0D0, immediately after it.
    virtual void entryHIO() {}

    /// @details 0x2B8. Defined inline: called from @ref doDelete and lands at
    /// 0x8002F3E0, immediately after it.
    virtual void removeHIO() {}

    // Nonvirtuals

    void createMdl();   ///< @unofficial Not virtual in this class, unlike daEnDpakkunBase_c's.
    void calcMdl();     ///< @unofficial
    void calcCcInfo();  ///< @unofficial Recomputes both colliders from the bone positions.
    void calcJnt();     ///< @unofficial Reads the bone matrices into the four position members.
    void setAnm(char *name, m3d::playMode_e playMode, float blendFrame); ///< @unofficial

    dHeapAllocator_c mAllocator;   ///< 0x524 @unofficial
    nw4r::g3d::ResFile mResFile;   ///< 0x540 @unofficial
    m3d::mdl_c mModel;             ///< 0x544 @unofficial
    m3d::anmChr_c mAnm;            ///< 0x584 @unofficial
    m3d::anmMatClr_c mClrAnm;      ///< 0x5BC The damage/freeze tint animation. @unofficial
    dCc_c mHeadCc;                 ///< 0x5E8 Fixed collider; its Y offset is negated when the plant is upside-down. @unofficial
    dCc_c mBodyCc;                 ///< 0x68C Collider stretched between #mStemPos and the actor position. @unofficial
    mVec3_c mHeadPos;              ///< 0x730 Bone 6, raised 12 units along its own Y axis. Initialised to mPos. @unofficial
    mVec3_c mStemPos;              ///< 0x73C Bone 6, raised 8 units along its own Y axis. @unofficial
    mVec3_c mNeckPos;              ///< 0x748 Bone 6, raised 12 units along its own Y axis (same construction as #mHeadPos). @unofficial
    mVec3_c mMouthPos;             ///< 0x754 World position of bone 8. @unofficial
    int mIsDying;                  ///< 0x760 Set once any lethal hit has been accepted; every later hitCallback returns early. @unofficial
    int mSakasa;                   ///< 0x764 Whether the plant grows downwards. @ref PARAM_Sakasa. @unofficial
    float mWaterCheckHeight;       ///< 0x768 Height handed to dEn_c::WaterCheck. Set to 1.0f by #create. @unofficial
    u8 m_76c[0x76E - 0x76C];       ///< 0x76C @unused
    bool mIsOffscreen;             ///< 0x76E Result of the last #ActorDrawCullCheck. @unofficial
    // 1 byte of trailing padding -> sizeof == 0x770

    ACTOR_PARAM_CONFIG(Sakasa, 0, 1); ///< @unofficial
};
