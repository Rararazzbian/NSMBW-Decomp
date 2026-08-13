#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_circle_light_mask.hpp>
#include <game/bases/d_bc.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>

/**
 * @brief The Propeller Block / "?" balloon that carries a player (@p EN_HATENA_BALLOON).
 * @details Spawns either as the balloon a player rides (@ref mBalloonType == 0) or as
 * an item balloon (@ref mBalloonType == 1, which additionally loads @ref mItemModel /
 * @ref mItemAnmChr from a second archive). It flies with the Wii Remote shake
 * (@ref remocon_shake_check, @ref remocon_speed_set), pops on damage
 * (@ref break_balloon, @ref break_effect) and hunts for a free spot to drop the
 * player into via @ref StateID_SearchSpace.
 *
 * @note This derives straight from @ref dEn_c and introduces **ZERO new virtual
 * functions**. Proof, taken from the bytes: @p __vt__19daEnHatenaBalloon_c
 * (@p .data 0x803236B0) and @p __vt__5dEn_c (@p .data 0x80311EE0) are both
 * exactly 0x280 bytes, and a word-for-word comparison over 0x000..0x27C differs
 * in exactly **24** slots, every one of which resolves to a
 * @p daEnHatenaBalloon_c symbol overriding the corresponding @p dEn_c /
 * @p fBase_c slot. There is no slot 0x280, so nothing new is added and the
 * declaration order of the overrides below is free
 * (see HANDOFF.md, "Overrides are a free lever -- the flush-order rule").
 *
 * The 24 overridden slots, in vtable order:
 * 0x008 @ref create, 0x014 @ref doDelete, 0x020 @ref execute, 0x02C @ref draw,
 * 0x030 @ref preDraw, 0x048 the destructor, 0x064 @ref block_hit_init,
 * 0x0F4 @ref Normal_VsEnHitCheck, 0x0F8 @ref Normal_VsPlHitCheck,
 * 0x0FC @ref Normal_VsYoshiHitCheck, 0x100 @ref hitCallback_Star,
 * 0x104 @ref hitCallback_Slip, 0x108 @ref hitCallback_Large,
 * 0x10C @ref hitCallback_Spin, 0x114 @ref hitCallback_WireNet,
 * 0x118 @ref hitCallback_HipAttk, 0x11C @ref hitCallback_YoshiHipAttk,
 * 0x128 @ref hitCallback_Cannon, 0x12C @ref hitCallback_Shell,
 * 0x130 @ref hitCallback_Fire, 0x134 @ref hitCallback_Ice,
 * 0x138 @ref hitCallback_YoshiBullet, 0x150 @ref isQuakeDamage,
 * 0x154 @ref hitYoshiEat.
 *
 * @note There is no @p __ct__19daEnHatenaBalloon_cFv anywhere in @p wiimj2d.dol:
 * the constructor is inlined into @p daEnHatenaBalloon_c_classInit, which is the
 * FIRST function of the TU (0x801102B0). Do NOT define one out of line.
 * @p classInit is a plain file-scope function
 * (@p daEnHatenaBalloon_c_classInit__Fv, no class qualification in the mangled
 * name) and it is where @p li r3, 0x8a0 -- the class's @p sizeof -- appears.
 *
 * @note The six states are plain @p sFStateID_c (0x30 bytes each in @p .bss, and
 * @p __vt__34sFStateID_c<19daEnHatenaBalloon_c> exists while no
 * @p sFStateVirtualID_c instantiation does), so use @p STATE_FUNC_DECLARE /
 * @p STATE_DEFINE and expect **no** @p baseID_* blocks at the head of the TU.
 *
 * @note One function in the @p .text range has no symbol: @p fn_80112040
 * (0x80112040, 0x88 bytes). It takes the actor in @p r3, reads only @p mPos.x
 * (0xAC) and returns a float from @p dBgParameter_c::getLoopScrollDispPosX, so
 * it is a **file-static free function**, not a member -- do not declare it here.
 * It sits between @ref pause_check and @ref shake_disp_check in @p .text.
 * @statetable
 * @unofficial
 */
class daEnHatenaBalloon_c : public dEn_c {
public:
    /// @brief Destroys the balloon.
    /// @details **Declare it here, define it OUT OF LINE, and make it the LAST
    /// function in d_a_en_hatena_balloon.cpp.** It is at 0x80114480 (0xF8
    /// bytes), immediately before @p __sinit_d_a_en_hatena_balloon_cpp at
    /// 0x80114580, and nothing else of the class follows it. Defining it inline
    /// in this header misorders the whole trailing flush block and no
    /// per-function diff can see that -- see HANDOFF.md and the same shape in
    /// @p d_a_en_blockmain.cpp / @p d_a_en_bros_base.cpp.
    /// @note Its 0xF8 bytes are the member teardown chain: @ref mItemAnmTexPat
    /// (this + 0x730), @ref mMaskAllocator (this + 0x834) and the rest of the
    /// m3d members, then @p dEn_c's destructor.
    virtual ~daEnHatenaBalloon_c();

    // ------------------------------------------------------------------
    // Overrides of dEn_c / fBase_c virtuals. NO new virtuals exist in this
    // class, so every slot below is an inherited one and this ordering is a
    // free lever. It is written in .text definition order.
    // ------------------------------------------------------------------

    virtual int create();   ///< [vt 0x008] 0x80110410. Reads the three actor-parameter nybbles, picks the player, builds the models and registers @p mBc / @p mCc.
    virtual int execute();  ///< [vt 0x020] 0x80110720.
    virtual int preDraw();  ///< [vt 0x030] 0x80110910.
    virtual int draw();     ///< [vt 0x02C] 0x80110A10.
    virtual int doDelete(); ///< [vt 0x014] 0x80110C40. 8 bytes.

    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);    ///< [vt 0x0F8] 0x80110F20.
    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other); ///< [vt 0x0FC] 0x80111000.
    virtual void Normal_VsEnHitCheck(dCc_c *self, dCc_c *other);    ///< [vt 0x0F4] 0x801110E0.

    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);          ///< [vt 0x130] 0x801112F0.
    virtual bool hitCallback_YoshiBullet(dCc_c *self, dCc_c *other);   ///< [vt 0x138] 0x801113D0. 8 bytes (`li r3,0; blr`).
    virtual bool hitCallback_Cannon(dCc_c *self, dCc_c *other);        ///< [vt 0x128] 0x801113E0. 0x24 bytes.
    virtual bool hitCallback_Ice(dCc_c *self, dCc_c *other);           ///< [vt 0x134] 0x80111410. **4 bytes -- a bare `blr`, i.e. an empty body with no return statement.** Must be defined out of line.
    virtual bool hitCallback_Star(dCc_c *self, dCc_c *other);          ///< [vt 0x100] 0x80111420. 0x24 bytes.
    virtual bool hitCallback_Large(dCc_c *self, dCc_c *other);         ///< [vt 0x108] 0x80111450. 0x24 bytes.
    virtual bool hitCallback_Spin(dCc_c *self, dCc_c *other);          ///< [vt 0x10C] 0x80111480. 0x24 bytes.
    virtual bool hitCallback_Shell(dCc_c *self, dCc_c *other);         ///< [vt 0x12C] 0x801114B0. 0x1C8 bytes, the largest of the group.
    virtual bool hitCallback_Slip(dCc_c *self, dCc_c *other);          ///< [vt 0x104] 0x80111680. 0x24 bytes.
    virtual bool hitCallback_WireNet(dCc_c *self, dCc_c *other);       ///< [vt 0x114] 0x801116B0. 8 bytes.
    virtual bool hitCallback_HipAttk(dCc_c *self, dCc_c *other);       ///< [vt 0x118] 0x801116C0.
    virtual bool hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other);  ///< [vt 0x11C] 0x801117A0.

    virtual void hitYoshiEat(dCc_c *self, dCc_c *other); ///< [vt 0x154] 0x801118C0. 0x10 bytes.
    virtual void block_hit_init();                       ///< [vt 0x064] 0x801118D0. **4 bytes -- empty. Must be defined out of line.**
    virtual BOOL isQuakeDamage();                        ///< [vt 0x150] 0x80114470. 8 bytes.

    // ------------------------------------------------------------------
    // Non-virtual member functions, in .text order. Every one of these takes
    // `this` in r3 with the declared arguments starting in r4 -- checked
    // against the disassembly, so none of them is static.
    // ------------------------------------------------------------------

    /// @brief 0x80110C50, 0x4C bytes. @unofficial
    bool ccLineCheck(float a, float b);

    /// @brief 0x80110CA0, 0x138 bytes. Rebuilds the collider line from the
    /// balloon's current size / facing. @unofficial
    void setCcLine();

    /// @brief 0x80110DE0, 0x13C bytes. Shared player/Yoshi hit body. @unofficial
    /// @note This is a MEMBER, not a static: the disassembly consumes r3, r4 and
    /// r5, and the callers pass @p this in r3, @p other->mOwner in r4 and
    /// @p self->mOwner (i.e. @p this again, as a @p dActor_c *) in r5. The
    /// second argument really is redundant in the original source.
    void PlYsHitCheck(dActor_c *player, daEnHatenaBalloon_c *self);

    /// @brief 0x801118E0, 0xB0 bytes. @unofficial
    /// @note Mangles @p FSc7mVec3_c -- @p s8 then @p mVec3_c **by value**.
    void hipattackhit(s8 dir, mVec3_c pos);

    /// @brief 0x80111990, 0x414 bytes. Builds every model / animation member.
    /// @details Order, and it is the order the members are declared in:
    /// frame heap from #mAllocator, #mResFile, #mModel + #mAnmChr + #mAnmTexSrt
    /// + #mAnmTexPat, then #mBalloonModel + #mBalloonAnmChr + #mBalloonAnmTexPat,
    /// then -- only when #mBalloonType == 1 -- #mItemModel + #mItemAnmChr.
    /// @unofficial
    void model_set();

    void item_draw_calc(mVec3_c *out); ///< 0x80111DB0, 0x10C bytes. Writes #mItemDrawPos. @unofficial
    void anm_set(int anmNo);           ///< 0x80111EC0, 0xCC bytes. Stores @p anmNo in #mAnmNo. @unofficial
    void pause_check();                ///< 0x80111F90, 0xA8 bytes. @unofficial
    void shake_disp_check();           ///< 0x801120D0, 0x34 bytes. @unofficial
    void createItem();                 ///< 0x80112110, 0xAC bytes. @unofficial
    void break_balloon(s16 mode);      ///< 0x801121C0, 0xA0 bytes. Mangles @p Fs -- a `short`, not an `int`. @unofficial
    void player_set();                 ///< 0x80112260, 0x8C bytes. @unofficial

    /// @brief 0x801122F0, 0x28C bytes. @unofficial
    /// @note Mangles @p FRC7mVec3_cUlUlUl. The three trailing arguments are
    /// `unsigned long`, NOT `u32`: `u32` is `unsigned int` and mangles @p Ui,
    /// naming a symbol that does not exist. The emitted words are identical
    /// either way, so only a callee-symbol comparison catches it.
    u32 pointBgCheck(const mVec3_c &pos, unsigned long a, unsigned long b, unsigned long c);

    void goalpole_check();             ///< 0x80112580, 0x3C bytes. @unofficial
    void floor_check();                ///< 0x801125C0, 0x214 bytes. @unofficial
    void all_bgcheck(u8 &result);      ///< 0x801127E0, 0x164 bytes. Walks #s_someCheckData 4x2 and calls @ref pointBgCheck for each entry. @unofficial
    void fly_yspeed_set();             ///< 0x80112950, 0x1AC bytes. @unofficial
    void fly_xspeed_set(bool a);       ///< 0x80112B00, 0x164 bytes. @unofficial
    void fly_ydisp_check(bool a);      ///< 0x80112C70, 0xDC bytes. @unofficial
    void fly_xdisp_check(bool a);      ///< 0x80112D50, 0x198 bytes. @unofficial
    void fly_dispin_check();           ///< 0x80112EF0, 0xC8 bytes. @unofficial
    void escape_dispout_check();       ///< 0x80112FC0, 0xCC bytes. @unofficial
    void remocon_speed_set();          ///< 0x80113090, 0x308 bytes. @unofficial
    void break_speed_set();            ///< 0x801133A0, 0x58 bytes. @unofficial
    void remocon_times_check();        ///< 0x80113400, 0x54 bytes. @unofficial
    void player_out_check();           ///< 0x80113460, 0x8C bytes. @unofficial
    void remocon_shake_check();        ///< 0x801134F0, 0xC0 bytes. @unofficial
    void ButtonPlayerColSet();         ///< 0x801135B0, 0x18 bytes. @unofficial
    void break_effect();               ///< 0x801135D0, 0x6C bytes. @unofficial
    void dispInFlyInitCheck(int mode); ///< 0x80113640, 0x100 bytes. @unofficial
    void create_wait_pos_set();        ///< 0x80113740, 0x190 bytes. @unofficial

    // ------------------------------------------------------------------
    // States. Plain sFStateID_c -- use STATE_DEFINE, not STATE_VIRTUAL_DEFINE.
    // The .bss objects are 0x30 bytes each and sit in this order.
    // ------------------------------------------------------------------

    STATE_FUNC_DECLARE(daEnHatenaBalloon_c, DispFlyWait);  ///< StateID @p .bss 0x80375408. Off-screen, waiting to fly in.
    STATE_FUNC_DECLARE(daEnHatenaBalloon_c, DispFlyMove);  ///< StateID @p .bss 0x80375448. Flying in towards the screen.
    STATE_FUNC_DECLARE(daEnHatenaBalloon_c, Fly);          ///< StateID @p .bss 0x80375488. Normal remote-controlled flight.
    STATE_FUNC_DECLARE(daEnHatenaBalloon_c, Escape);       ///< StateID @p .bss 0x803754C8. Leaving the screen.
    STATE_FUNC_DECLARE(daEnHatenaBalloon_c, HipAttack);    ///< StateID @p .bss 0x80375508. Initialize/finalize are both 4-byte empties.
    STATE_FUNC_DECLARE(daEnHatenaBalloon_c, SearchSpace);  ///< StateID @p .bss 0x80375548. Looking for free ground to drop the player onto.

    // ------------------------------------------------------------------
    // Static data members.
    // ------------------------------------------------------------------

    /// @brief The probe table @ref all_bgcheck walks: 4 rows of 0x14 bytes.
    /// @details @p .rodata 0x802F4E20, 0x50 bytes. Each row is two {x, y}
    /// offset pairs followed by a 32-bit mask that is @p and-ed with the
    /// running result and OR-ed back a byte at a time. The loop is
    /// `for (i = 0; i < 4; i++) for (j = 0; j < 2; j++)`, stepping the inner
    /// pointer by 8 and the outer by 0x14. @unofficial
    struct checkData_s {
        float mOffsetX; ///< 0x00
        float mOffsetY; ///< 0x04
        float mOffsetX2; ///< 0x08
        float mOffsetY2; ///< 0x0C
        u32 mFlag;      ///< 0x10 Read with @p lwz, used both as a full-word mask and as a byte.
    };
    static const checkData_s s_someCheckData[4];

    /// @brief @p .bss 0x80375578 / 0x80375584 / 0x80375590, 0xC bytes each.
    /// @details Nothing in this TU's @p .text references them -- they are
    /// zero-initialised @p .bss and are read from elsewhere -- but they are this
    /// class's statics and must be defined here or the @p .bss range breaks.
    /// @unofficial
    static mVec3_c sm_bg_check_size_mame;
    static mVec3_c sm_bg_check_size_normal; ///< @copydoc sm_bg_check_size_mame
    static mVec3_c sm_bg_check_size_super;  ///< @copydoc sm_bg_check_size_mame

    /// @brief HIO-tunable constants. All five are non-const @p float in
    /// @p .sdata (0x80429570..0x80429580, in this order), so they are read
    /// through @p @sda21 and must be defined with a non-zero initialiser.
    /// @unofficial
    static float sm_hio_gravity;         ///< @p .sdata 0x80429570. Stored into @p mAccel.y by @ref create.
    static float sm_hio_base_fly_timer_x; ///< @p .sdata 0x80429574. Used by @ref executeState_Fly.
    static float sm_hio_fly_yspeed;      ///< @p .sdata 0x80429578. Used by @ref fly_yspeed_set and @ref initializeState_Fly.
    static float sm_hio_mask_size;       ///< @p .sdata 0x8042957C. Radius handed to @ref mLightMask.
    static float sm_hio_mask_y_diff;     ///< @p .sdata 0x80429580. Y offset of @ref mLightMask from @p mPos.

    // ------------------------------------------------------------------
    // Actor parameters. create() reads mParam once and splits it into three
    // nybbles:
    //   clrlwi  r8, r4, 28        -> mParam & 0xF        -> mPlayerNo   (0x810)
    //   extrwi  r7, r4, 4, 24     -> (mParam >> 4) & 0xF -> m_814       (0x814)
    //   extrwi. r0, r4, 4, 20     -> (mParam >> 8) & 0xF -> mBalloonType(0x7EC)
    // ------------------------------------------------------------------

    ACTOR_PARAM_CONFIG(PLAYER_NO, 0, 4);    ///< @unofficial Which player this balloon belongs to.
    ACTOR_PARAM_CONFIG(SUB_TYPE, 4, 4);     ///< @unofficial Compared against 2 by @ref create. @unofficial
    ACTOR_PARAM_CONFIG(BALLOON_TYPE, 8, 4); ///< @unofficial 0 = ridden balloon, 1 = item balloon.

    // ------------------------------------------------------------------
    // Members.
    //
    // daEnHatenaBalloon_c's own data begins at 0x524: sizeof(dEn_c) is 0x528 but
    // its DATA size is 0x524, and MWCC reuses a base class's tail padding --
    // exactly as in daEnBrosBase_c, daEnBlockMain_c and daEnJimenPakkunBase_c.
    //
    // 0x524..0x75C is read directly out of the inlined constructor in
    // daEnHatenaBalloon_c_classInit, which calls each member's constructor at a
    // known offset -- that fixes the whole model block with no guessing. The
    // sensor triple and everything after it comes from create() / model_set() /
    // the state bodies.
    //
    // The layout ends at 0x89C; sizeof is 0x8A0 because the class inherits
    // dEn_c's 8-byte alignment (dEn_c is itself 0x524 of data in 0x528). There
    // is NO member at 0x89C.
    // ------------------------------------------------------------------

    dHeapAllocator_c mAllocator;         ///< [0x524] Frame heap for the models. @p __ct__16dHeapAllocator_cFv at this+0x524 in classInit. @unofficial
    nw4r::g3d::ResFile mResFile;         ///< [0x540] Zeroed by the constructor; filled by @ref model_set. @unofficial
    m3d::mdl_c mModel;                   ///< [0x544] The main balloon model. @unofficial
    m3d::mdl_c mBalloonModel;            ///< [0x584] The second model, from the profile's 0x20 name. @unofficial
    m3d::anmChr_c mAnmChr;               ///< [0x5C4] Chr animation of #mModel. @unofficial
    m3d::anmChr_c mBalloonAnmChr;        ///< [0x5FC] Chr animation of #mBalloonModel. @unofficial
    m3d::anmTexPat_c mAnmTexPat;         ///< [0x634] TexPat animation of #mModel; frame 0 or 1 depending on @p dActorMng_c::envAllWaterCheck. @unofficial
    m3d::anmTexPat_c mBalloonAnmTexPat;  ///< [0x660] TexPat animation of #mBalloonModel, same water switch. @unofficial
    m3d::anmTexSrt_c mAnmTexSrt;         ///< [0x68C] TexSrt animation of #mModel. @unofficial
    m3d::mdl_c mItemModel;               ///< [0x6B8] Only created when #mBalloonType == 1. @unofficial
    m3d::anmChr_c mItemAnmChr;           ///< [0x6F8] Chr animation of #mItemModel. @unofficial
    m3d::anmTexPat_c mItemAnmTexPat;     ///< [0x730] Constructed unconditionally, driven only in the item case. @unofficial

    /// @brief [0x75C] The three sensors handed to @p mBc.
    /// @details @ref create fills them with
    /// { 0x80000001, -0x7000, 0x7000, 0x3000 },
    /// { 0x80000001, -0x7000, 0x7000, 0x1D000 } and
    /// { 0x80000001, 0x5000, 0x18000, 0x7000 }, then calls
    /// @p dBc_c::set(this, &mSensorFoot, &mSensorHead, &mSensorWall). @unofficial
    sBcSensorLine mSensorFoot;
    sBcSensorLine mSensorHead; ///< [0x76C] @copydoc mSensorFoot
    sBcSensorLine mSensorWall; ///< [0x77C] @copydoc mSensorFoot

    mVec3_c m_78c;      ///< [0x78C] Written by @ref draw. @unofficial
    mVec3_c m_798;      ///< [0x798] Written by @ref initializeState_DispFlyWait. @unofficial
    mVec3_c mItemDrawPos; ///< [0x7A4] Zeroed by @ref create when #mBalloonType == 1; produced by @ref item_draw_calc. @unofficial
    mVec3_c m_7b0;      ///< [0x7B0] Touched by @ref PlYsHitCheck, @ref break_speed_set, @ref hitCallback_Shell and @ref executeState_SearchSpace. @unofficial
    u8 m_7bc[0x7C4 - 0x7BC]; ///< [0x7BC] @unused -- nothing in the TU reads or writes these 8 bytes.
    float m_7c4;        ///< [0x7C4] @ref draw / @ref pause_check. @unofficial
    float m_7c8;        ///< [0x7C8] @ref draw / @ref shake_disp_check. @unofficial
    float m_7cc;        ///< [0x7CC] The hottest float in the class (21 accesses): flight Y speed, driven by @ref fly_yspeed_set, @ref remocon_speed_set and the Fly/Escape states. @unofficial
    float m_7d0;        ///< [0x7D0] Read by @ref draw only. @unofficial
    mVec3_c m_7d4;      ///< [0x7D4] Filled by @ref create from a 3-entry @p .bss table indexed by the player's size, and read back by @ref executeState_SearchSpace. @unofficial
    int m_7e0;          ///< [0x7E0] @ref initializeState_SearchSpace / @ref remocon_shake_check. @unofficial
    int m_7e4;          ///< [0x7E4] @ref remocon_shake_check / @ref shake_disp_check. @unofficial
    int mAnmNo;         ///< [0x7E8] Written only by @ref anm_set. @unofficial
    int mBalloonType;   ///< [0x7EC] @p ACTOR_PARAM(BALLOON_TYPE). 1 selects the item balloon; 13 reads across the TU. @unofficial
    int m_7f0;          ///< [0x7F0] @ref create_wait_pos_set / @ref initializeState_DispFlyWait. @unofficial
    int m_7f4;          ///< [0x7F4] @ref initializeState_DispFlyWait / @ref executeState_Escape. @unofficial
    int m_7f8;          ///< [0x7F8] @ref remocon_times_check. @unofficial
    int m_7fc;          ///< [0x7FC] Set to 0x28 by @ref create when #mBalloonType == 0 and @p ACTOR_PARAM(SUB_TYPE) == 2; also @ref ButtonPlayerColSet. @unofficial
    int m_800;          ///< [0x800] Zeroed by @ref create. @unofficial
    int m_804;          ///< [0x804] @ref initializeState_SearchSpace / @ref executeState_SearchSpace. @unofficial
    int m_808;          ///< [0x808] @ref execute. @unofficial
    u8 m_80c;           ///< [0x80C] Zeroed by @ref create in the #mBalloonType == 1 branch. @unofficial
    u8 m_80d[0x810 - 0x80D]; ///< [0x80D] @unused
    int mPlayerNo;      ///< [0x810] @p ACTOR_PARAM(PLAYER_NO); fed straight to @p daPyMng_c::getPlayer. @unofficial
    int m_814;          ///< [0x814] @p ACTOR_PARAM(SUB_TYPE). @unofficial
    u16 m_818;          ///< [0x818] @ref execute only. @unofficial
    u8 m_81a;           ///< [0x81A] @unused
    u8 m_81b;           ///< [0x81B] @ref execute only. @unofficial
    u8 m_81c;           ///< [0x81C] @unused
    u8 m_81d;           ///< [0x81D] @ref pause_check. @unofficial
    u8 m_81e;           ///< [0x81E] @ref fly_yspeed_set / @ref initializeState_Fly. @unofficial
    s8 m_81f;           ///< [0x81F] Set to -1 by @ref create (`li r6,-1; stb r6`), read by @ref break_balloon -- so it is SIGNED. @unofficial
    u8 m_820;           ///< [0x820] @ref fly_yspeed_set / @ref initializeState_Fly. @unofficial
    u8 m_821;           ///< [0x821] Facing / launch direction: @ref create writes 1, or 2 when the owning player's @p mSpeed.x is negative. @unofficial
    u8 m_822;           ///< [0x822] Set to 1 on the very first instruction of @ref create; read by @ref preDraw, @ref player_set and @ref player_out_check. @unofficial
    u8 m_823[0x828 - 0x823]; ///< [0x823] @unused
    mVec3_c m_828;      ///< [0x828] @ref hitCallback_YoshiHipAttk / @ref executeState_SearchSpace. @unofficial
    mHeapAllocator_c mMaskAllocator; ///< [0x834] Constructed in classInit and handed to @ref mLightMask's init as `init(&mMaskAllocator, 2)`. @unofficial
    dCircleLightMask_c mLightMask;   ///< [0x850] Confirmed exactly: classInit stores @p __vt__18dCircleLightMask_c at 0x850 and then the inlined @p reset() writes 0.0f/0/0 at 0x860/0x864/0x868, which is @p mRadius/@p mMask/@p mQuad of a 0x1C-byte object based at 0x850. @unofficial
    u8 mHitFlag;        ///< [0x86C] Raised to 1 by every hit path (@ref Normal_VsPlHitCheck, @ref hitCallback_Fire, @ref hitCallback_Shell, ...); consumed by @ref executeState_SearchSpace. @unofficial
    u8 m_86d[0x870 - 0x86D]; ///< [0x86D] @unused
    mVec3_c mHitPos;    ///< [0x870] @p dBaseActor_c::getCenterPos of whatever hit the balloon, stored alongside #mHitFlag. @unofficial
    int m_87c;          ///< [0x87C] Gate checked with `> 0` at the top of the hit checks; zeroed by @ref create. @unofficial
    u8 m_880;           ///< [0x880] Second gate in the hit checks; zeroed by @ref create. @unofficial
    u8 m_881;           ///< [0x881] Zeroed by @ref create; @ref executeState_SearchSpace / @ref finalizeState_SearchSpace. @unofficial
    u8 m_882[0x884 - 0x882]; ///< [0x882] @unused
    mVec3_c m_884;      ///< [0x884] @ref executeState_SearchSpace. @unofficial
    mVec3_c m_890;      ///< [0x890] @ref executeState_SearchSpace / @ref finalizeState_SearchSpace. @unofficial
    // 0x890 + 0xC = 0x89C, rounded up to the class's 8-byte alignment ->
    // sizeof(daEnHatenaBalloon_c) == 0x8A0, which is exactly the `li r3, 0x8a0`
    // in daEnHatenaBalloon_c_classInit.
};
