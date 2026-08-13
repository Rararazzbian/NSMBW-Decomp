#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_effect.hpp>

/**
 * @brief Base implementation of the Koopa "Bros." enemies.
 * @details The shared walk / turn / throw / jump behaviour behind
 * @p EN_HAMMERBROS, @p EN_BOOMERANGBROS, @p EN_FIREBROS, @p EN_ICEBROS,
 * @p EN_HIMANBROS and @p EN_LIFT_HAMMERBROS, all of which live in
 * @p d_enemiesNP.rel. The projectile itself is left entirely to the subclasses,
 * which supply @ref weaponCreate, @ref weaponAttack, @ref setWeaponPos and the
 * two animation-frame getters.
 *
 * @note This derives straight from @ref dEn_c, @em not from @ref daEnCarry_c.
 * Proof: @p __vt__5dEn_c is exactly 0x280 bytes and every slot of
 * @p __vt__14daEnBrosBase_c in 0x008..0x27C is either byte-identical to
 * @p dEn_c's or a @p daEnBrosBase_c override of a @p dEn_c slot; the first new
 * slot is at 0x280. @p daEnCarry_c is ruled out because its own vtable (0x28C
 * bytes) already occupies 0x280/0x284/0x288 with
 * @p initializeState_Carry / @p executeState_Carry / @p finalizeState_Carry,
 * whereas this class puts @ref initializeState_Move there.
 *
 * @note There is no @p __ct__14daEnBrosBase_cFv anywhere in @p wiimj2d.dol, so
 * the constructor is inline and its only surviving copies are in the derived
 * actors' translation units inside @p d_enemiesNP.rel. Do NOT define one out of
 * line here. That constructor is also where @ref nodeCallback_c::mpOwner has to
 * be set: nothing in @p d_a_en_bros_base.cpp writes it, yet
 * @ref nodeCallback_c::timingA reads it.
 *
 * @note This class reads no actor parameters at all -- @ref create contains no
 * @p mParam access and there is no @p extrwi / @p clrlslwi against it anywhere
 * in the TU, so there are deliberately no @p ACTOR_PARAM_CONFIG entries. The
 * subclasses own the parameter layout.
 * @statetable
 * @unofficial
 */
class daEnBrosBase_c : public dEn_c {
public:
    /// @brief Per-node animation callback: applies the two extra arm/hand
    /// rotations that the throw states drive.
    /// @details Bound to #mModel by @ref create. @ref timingA adds
    /// #mArmAngle to node 0x0B's Z rotation and #mHandAngle to node 0x0E's.
    /// @p __vt__Q214daEnBrosBase_c14nodeCallback_c is 0x18 bytes: dtor,
    /// @ref timingA, then @p m3d::mdl_c::callback_c's inherited @p timingB and
    /// @p timingC.
    /// @unofficial
    class nodeCallback_c : public m3d::mdl_c::callback_c {
    public:
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl);

        /// @brief The actor this callback belongs to. [nodeCallback_c + 0x4]
        /// @details Nothing in @p d_a_en_bros_base.cpp writes it, so it is set
        /// by the (inline, never emitted in this TU) constructor of the owning
        /// actor. @ref timingA reads it as @p lwz r4, 0x4(r3).
        daEnBrosBase_c *mpOwner;
    };

    /// @brief Destroys the enemy.
    /// @details Defined inline. It is never called in this TU, so it lands in
    /// the end-of-TU flush block -- at 0x80025E90, the very first entry of it,
    /// because @p daEnBrosBase_c is the last class parsed and groups come out
    /// in reverse parse order. It is also the only member of this class in that
    /// block, so its declaration position is unconstrained. The weak dtors that
    /// follow it (@p mEf::levelEffect_c, @p mEf::effect_c, @p nodeCallback_c,
    /// @p m3d::mdl_c::callback_c, @p m3d::anmChr_c, then @p timingC / @p timingB)
    /// are the cascade this body pulls in.
    virtual ~daEnBrosBase_c();

    // ------------------------------------------------------------------
    // Base class overrides.
    // An override inherits its slot from dEn_c, so the order of THIS section is
    // free as far as the vtable is concerned. It would fix the order of the
    // end-of-TU inline flush block -- but the destructor above is this class's
    // only inline member, so there is nothing to order. Every function below is
    // defined OUT OF LINE in d_a_en_bros_base.cpp: each one sits at a natural
    // source position between other real functions, not in the trailing block.
    // ------------------------------------------------------------------

    virtual int create();                                       ///< vt 0x008
    virtual int doDelete();                                     ///< vt 0x014
    virtual int execute();                                      ///< vt 0x020
    virtual int draw();                                         ///< vt 0x02C
    virtual void finalUpdate();                                 ///< vt 0x05C

    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);    ///< vt 0x0F8
    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other); ///< vt 0x0FC

    virtual bool hitCallback_Spin(dCc_c *self, dCc_c *other);         ///< vt 0x10C
    virtual bool hitCallback_HipAttk(dCc_c *self, dCc_c *other);      ///< vt 0x118
    virtual bool hitCallback_Ice(dCc_c *self, dCc_c *other);          ///< vt 0x134
    virtual bool hitCallback_YoshiBullet(dCc_c *self, dCc_c *other);  ///< vt 0x138

    /// @brief Being squished by a jump.
    /// @note An override of @ref dEn_c's state of the same name: it takes
    /// dEn_c's slots 0x16C/0x170/0x174 and declares a @p StateID_DieFumi that
    /// SHADOWS dEn_c's. Any reference to the base state from code in this class
    /// must be written @p &dEn_c::StateID_DieFumi. This is what emits the
    /// @p baseID_DieFumi<5dEn_c> weak instantiation at 0x800268C0.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBrosBase_c, DieFumi);

    /// @brief Falling out of the screen.
    /// @note Same shadowing rule as @ref StateID_DieFumi; slots
    /// 0x178/0x17C/0x180, and @p baseID_DieFall<5dEn_c> at 0x800268B0.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBrosBase_c, DieFall);

    virtual bool setDamage(dActor_c *actor);                    ///< vt 0x220
    virtual bool createIceActor();                              ///< vt 0x22C
    virtual void setIceAnm();                                   ///< vt 0x230
    virtual void returnState_Ice();                             ///< vt 0x238

    // ------------------------------------------------------------------
    // New virtual functions (vtable 0x280..0x320).
    // The order of THIS section is fixed by the vtable -- do not reorder.
    // ------------------------------------------------------------------

    STATE_VIRTUAL_FUNC_DECLARE(daEnBrosBase_c, Move);      ///< 0x280 Walking and looking for an opening.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBrosBase_c, Attack);    ///< 0x28C Throwing while on the ground.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBrosBase_c, JumpSt);    ///< 0x298 Crouching before a jump.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBrosBase_c, Jump);      ///< 0x2A4 Airborne.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBrosBase_c, JumpEd);    ///< 0x2B0 Landing.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBrosBase_c, AirAttack); ///< 0x2BC Throwing while airborne.

    virtual float getCreateWeaponFrm() const; ///< 0x2C8 Animation frame at which @ref weaponCreate fires. Returns 0.0f here.
    virtual float getAttackFrm() const;       ///< 0x2CC Animation frame at which @ref weaponAttack fires. Returns 0.0f here.
    virtual void setSpeed();                  ///< 0x2D0 Empty here.
    virtual bool checkAtkArea();              ///< 0x2D4 Returns true here.
    virtual int getColor();                   ///< 0x2D8 Texture-pattern frame; returns -1 here.
    virtual void setAtkTimer();               ///< 0x2DC Empty here.
    virtual mVec3_c getAdjustOffset();        ///< 0x2E0 Returns a zero vector here. @note Returns by value: the body writes 0.0f to 0x0/0x4/0x8 of the sret pointer in r3, with @p this in r4.
    virtual void calcMoveDir();               ///< 0x2E4 Flips #mMoveDir and calls @ref setSpeed when the walk range is exhausted.
    virtual void setWeaponPos();              ///< 0x2E8 Empty here.
    virtual void weaponCreate();              ///< 0x2EC Empty here.
    virtual void weaponAttack();              ///< 0x2F0 Empty here.
    virtual void setJumpCnt();                ///< 0x2F4 Empty here.
    virtual void setJump();                   ///< 0x2F8 Empty here.
    virtual void beginJump();                 ///< 0x2FC Calls @ref setJump then changes to @ref StateID_AirAttack.
    virtual bool isInvalidBg();               ///< 0x300 Returns false here.
    virtual bool isAttackOK() const;          ///< 0x304 Returns true here.
    virtual void beginAttk();                 ///< 0x308 Sets #mAttackCnt to 1 and changes to @ref StateID_Attack.
    virtual void setAttackAnm();              ///< 0x30C
    virtual void initMoveCnt();               ///< 0x310 Sets #mMoveCnt to 0.
    virtual void initPosLv();                 ///< 0x314 Empty here.
    virtual void initType();                  ///< 0x318 Empty here.
    virtual void entryHIO();                  ///< 0x31C Empty here.
    virtual void removeHIO();                 ///< 0x320 Empty here.

    // ------------------------------------------------------------------
    // Nonvirtuals
    // ------------------------------------------------------------------

    void createMdl();     ///< @unofficial Loads the resource file, model, chr and texpat animations.
    void calcMdl();       ///< @unofficial
    void calcJntMtx();    ///< @unofficial Reads bones 6, 4, 0, 14 and 11 into the five matrices.
    bool calcTurnAngle(); ///< @unofficial Steps the Y angle towards the facing direction; true once it has arrived.
    void landonEffect();  ///< @unofficial
    void setAnm(char *name, m3d::playMode_e playMode, float blendFrame); ///< @unofficial
    void setMoveAnm(float blendFrame); ///< @unofficial Picks the forwards or backwards walk animation from #mAnmDir.
    void dirProc();       ///< @unofficial

    // ------------------------------------------------------------------
    // Members. Offsets verified against the destructor's teardown order,
    // create/createMdl/calcMdl/calcJntMtx and the state bodies.
    // ------------------------------------------------------------------

    dHeapAllocator_c mAllocator;              ///< [0x524] @unofficial
    nw4r::g3d::ResFile mResFile;              ///< [0x540] @unofficial
    m3d::mdl_c mModel;                        ///< [0x544] @unofficial
    m3d::anmChr_c mAnmChr;                    ///< [0x584] @unofficial
    nw4r::g3d::ResAnmTexPat mResAnmTexPat;    ///< [0x5BC] @unofficial
    m3d::anmTexPat_c mAnmTexPat;              ///< [0x5C0] Selects the colour variant; its frame is @ref getColor. @unofficial
    mMtx_c mJntMtx6;                          ///< [0x5EC] World matrix of node 6. @unofficial
    mMtx_c mJntMtx4;                          ///< [0x61C] World matrix of node 4. @unofficial
    mMtx_c mJntMtx0;                          ///< [0x64C] World matrix of node 0. @unofficial
    mMtx_c mJntMtx14;                         ///< [0x67C] World matrix of node 14. @unofficial
    mMtx_c mJntMtx11;                         ///< [0x6AC] World matrix of node 11. @unofficial
    int mAtkTimer;                            ///< [0x6DC] Counts down once the turn has finished and @ref isAttackOK passes; at 0 @ref executeState_Move calls @ref beginAttk. Filled in by @ref setAtkTimer. @unofficial
    int mJumpCnt;                             ///< [0x6E0] Frames until the next jump. Filled in by @ref setJumpCnt, then HALVED by @ref create for the first cycle; floored at 16 whenever the enemy is off the ground. @unofficial
    int mMoveCnt;                             ///< [0x6E4] Zeroed by @ref initMoveCnt; used by the subclasses. @unofficial
    int mAnmDir;                              ///< [0x6E8] 0 when #mMoveDir equals the facing direction (forwards walk anim), 1 otherwise (backwards walk anim). Drives @ref setMoveAnm. @unofficial
    int mStopXCnt;                            ///< [0x6EC] Frames left of the X-position freeze in @ref executeState_Move (the X is rewritten from the previous position each frame). @unofficial
    int mAttackCnt;                           ///< [0x6F0] Remaining throws in the current attack; set to 1 by @ref beginAttk. @unofficial
    int m_6f4;                                ///< [0x6F4] Zeroed by @ref initializeState_Move; used by the subclasses. @unofficial
    u8 m_6f8[0x6FC - 0x6F8];                  ///< [0x6F8] @unused
    float mMoveRangeFront;                    ///< [0x6FC] Distance ahead of #mHomePos the enemy may walk before turning. Set to 16.0f by @ref create. @unofficial
    float mMoveRangeBack;                     ///< [0x700] Distance behind #mHomePos the enemy may walk before turning. Set to 16.0f by @ref create. @unofficial
    s16 mArmAngle;                            ///< [0x704] Extra Z rotation applied to node 0x0B by @ref nodeCallback_c::timingA. Zeroed by @ref initializeState_AirAttack and @ref finalizeState_AirAttack. @unofficial
    s16 mHandAngle;                           ///< [0x706] Extra Z rotation applied to node 0x0E by @ref nodeCallback_c::timingA. @unofficial
    mVec3_c mHomePos;                         ///< [0x708] Spawn position, copied from @p mPos by @ref create; @ref calcMoveDir measures the walk range from it. @unofficial
    u8 mMoveDir;                              ///< [0x714] The direction the enemy is walking in. Initialised from @p getPl_LRflag and flipped by @ref calcMoveDir; compare with @p dActor_c's own direction at 0x348, which tracks the player every frame. @unofficial
    u8 m_715[0x71C - 0x715];                  ///< [0x715] @unused
    nodeCallback_c mNodeCallback;             ///< [0x71C] @unofficial
    mEf::levelEffect_c mEffect;               ///< [0x724] Not used anywhere in this TU other than the destructor; the subclasses drive it. @unofficial
    // 0x724 + sizeof(mEf::levelEffect_c) (0x128) = 0x84C, rounded up to the
    // class's 8-byte alignment -> sizeof(daEnBrosBase_c) == 0x850.
};
