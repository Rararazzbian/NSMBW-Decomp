#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_bg_ctr.hpp>

/**
 * @brief Base implementation of every "hittable block" actor.
 * @details Everything a block has in common: the @ref dBg_ctr_c tile registration
 * and its three hit callbacks (from below, from above, from the side), the pop-up
 * / pop-down state machine, the squash-and-stretch scale animation, and the very
 * large item-spawning menu (@ref itemkey_set, @ref jumpdai_set, @ref yossy_set,
 * @ref eggitem_set, @ref item_ivy_set and their multiplayer variants). Its one
 * subclass inside @p wiimj2d.dol is @ref daEnCoinMain_c
 * (@p __vt__14daEnCoinMain_c, also 0x2EC); the rest live in the RELs.
 *
 * @note This derives straight from @ref dEn_c. Proof, taken from the bytes:
 * @p __vt__5dEn_c is exactly 0x280 and @p __vt__15daEnBlockMain_c is 0x2EC, and
 * a word-for-word comparison of the two over 0x000..0x27C differs in
 * **exactly one slot** -- 0x048, the destructor. Every other inherited slot is
 * literally @p dEn_c's own pointer, so this class overrides nothing else and the
 * first new slot is 0x280. (@p dActorMultiState_c and @p dActor_c are ruled out
 * by the same comparison: their vtables are 0xE4 and 0xD4 and end long before
 * 0x280.)
 *
 * @note There is no @p create, @p execute, @p draw or @p doDelete here -- not
 * even as an override. This class is never a live actor on its own; the derived
 * blocks own the whole lifecycle and simply call into the helpers below.
 *
 * @note There is no @p __ct__15daEnBlockMain_cFv anywhere in @p wiimj2d.dol, so
 * the constructor is inline; do NOT define one out of line here.
 *
 * @note This class reads no actor parameters: there is no @p mParam access and
 * no @p extrwi / @p clrlslwi against one anywhere in @p d_a_en_blockmain.cpp, so
 * there are deliberately no @p ACTOR_PARAM_CONFIG entries. The subclasses own the
 * parameter layout.
 * @statetable
 * @unofficial
 */
class daEnBlockMain_c : public dEn_c {
public:
    /// @brief Destroys the block.
    /// @details **Define this OUT OF LINE, and make it the LAST function in
    /// d_a_en_blockmain.cpp.** It is at 0x80023340, immediately before
    /// @p __sinit_d_a_en_blockmain_cpp and after @ref block_downmove_diffend,
    /// and nothing weak follows it. Defining it inline in this header instead
    /// misorders the whole trailing flush block once @p d_a_player.hpp is in the
    /// include set, and no per-function diff can see that. See
    /// @p d_a_en_lkuribo_base.cpp and @p d_a_en_bros_base.cpp for the same shape.
    /// Its only work is #mBgCtr's destructor followed by @p dEn_c's.
    virtual ~daEnBlockMain_c();

    // ------------------------------------------------------------------
    // New virtual functions (vtable 0x280..0x2E8).
    // The order of THIS section is fixed by the vtable -- do not reorder.
    // There are no overrides of dEn_c virtuals other than the destructor above,
    // so nothing in this class has a free declaration position.
    //
    // The eight `initialize_* / block_*` hooks are all empty (4 bytes, `blr`)
    // here and none of them is called from a non-virtual site in this TU, so
    // every one of them must be defined OUT OF LINE or the slot breaks. Their
    // target addresses interleave with the state bodies (e.g. initialize_upmove
    // at 0x80022E80 sits between initializeState_UpMove and
    // finalizeState_UpMove), so write them in .text address order.
    // ------------------------------------------------------------------

    virtual void initialize_upmove();      ///< 0x280 Empty here. Called by @ref initializeState_UpMove and @ref initializeState_UpMove_Diff (tail call).
    virtual void initialize_downmove();    ///< 0x284 Empty here. Called by @ref initializeState_DownMove and @ref initializeState_DownMove_Diff (tail call).
    virtual void block_upmove();           ///< 0x288 Empty here. Called every frame by @ref executeState_UpMove.
    virtual void block_upmove_diff();      ///< 0x28C Empty here. Called by @ref executeState_UpMove_Diff once the rise has finished.
    virtual void block_downmove();         ///< 0x290 Empty here. Called every frame by @ref executeState_DownMove.
    virtual void block_downmove_end();     ///< 0x294 Empty here. Tail-called by @ref executeState_DownMoveEnd when #mEndTimer hits 0.
    virtual void block_downmove_diff();    ///< 0x298 Empty here. Called by @ref executeState_DownMove_Diff.
    virtual void block_downmove_diffend(); ///< 0x29C Empty here. Tail-called by @ref executeState_DownMove_DiffEnd when #mEndTimer hits 0.

    /// @brief 0x2A0. Steps the squash-and-stretch scale one frame.
    /// @details Grows @p mScale by #mScaleStep towards #mScaleMax while the block
    /// is moving the way @p mode says, otherwise shrinks it towards #mScaleMin
    /// and clears #m_67f. @p mScale.y is then copied from @p mScale.x. Called
    /// from the four moving states with @p mode = 0.
    virtual void block_scale_set(u8 mode);

    STATE_VIRTUAL_FUNC_DECLARE(daEnBlockMain_c, UpMove);            ///< 0x2A4 Block rising after being hit.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBlockMain_c, DownMove);          ///< 0x2B0 Block falling back down.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBlockMain_c, DownMoveEnd);       ///< 0x2BC Six-frame settle after landing; see #mEndTimer.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBlockMain_c, UpMove_Diff);       ///< 0x2C8 As @ref StateID_UpMove, but the motion accumulates into #mMoveDiff instead of moving the actor.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBlockMain_c, DownMove_Diff);     ///< 0x2D4 The @ref StateID_DownMove counterpart of the above.
    STATE_VIRTUAL_FUNC_DECLARE(daEnBlockMain_c, DownMove_DiffEnd);  ///< 0x2E0 The @ref StateID_DownMoveEnd counterpart of the above.

    // ------------------------------------------------------------------
    // Static members.
    //
    // CFront mangling does not mark static members, so these were classified
    // from the register conventions: in each of them r3 already holds a real
    // argument (the block, or the collider), not `this`. They are all used as
    // dBg_ctr_c / dCc_c callback pointers, which is why they have to be static.
    // ------------------------------------------------------------------

    /// @brief @unofficial The enemy half of the foot callback: an enemy is
    /// standing on the block when it pops.
    /// @details Scores it through @p dEnCombo_c::setScore, sets its death
    /// direction from @p getTrgToSrcDir_Main, and raises #m_667 for the two
    /// actor types that need the extra cry.
    static void common_callBackF_enemy(dActor_c *self, dActor_c *other);

    /// @brief @unofficial Shared body of the head callback: something bumped the
    /// block from below.
    /// @details Sets #m_64c when the "hard" bg flag is on, ORs bit 1 into #m_68a,
    /// resolves the player (following @p getRidePlayer through Yoshi), records it
    /// in #mHitPlayerH and sets #m_680 = 1.
    /// @note Returns the block, not void: callBackH/nomal_callBackH consume
    /// the returned pointer (`cmpwi r3,0; beq; stb r0,0x67f(r3)`). CFront
    /// mangling carries no return type, so this changes no symbol and does
    /// not disturb the proven vtable. @unofficial
    static daEnBlockMain_c *common_callBackH(dActor_c *self, dActor_c *other);
    static void callBackF(dActor_c *self, dActor_c *other);              ///< @unofficial
    static void nomal_callBackF(dActor_c *self, dActor_c *other);        ///< @unofficial
    static void callBackH(dActor_c *self, dActor_c *other);              ///< @unofficial
    static void nomal_callBackH(dActor_c *self, dActor_c *other);        ///< @unofficial
    static void side_block_moveset(daEnBlockMain_c *self, dActor_c *other, u8 dir); ///< @unofficial Sets #m_680 = 3, records the player in #mHitPlayerW / #m_674 and stores @p dir in #m_68d.
    static bool shell_callBackW(dActor_c *self, dActor_c *other, u8 dir); ///< @unofficial True when the hit was consumed as a shell hit.
    static void callBackW(dActor_c *self, dActor_c *other, u8 dir);       ///< @unofficial
    static void obj_callBackW(dActor_c *self, dActor_c *other, u8 dir);   ///< @unofficial Empty (4 bytes).
    static void enemy_only_callBackW(dActor_c *self, dActor_c *other, u8 dir); ///< @unofficial
    static void playeronly_callBackF(dActor_c *self, dActor_c *other);    ///< @unofficial Empty (4 bytes).
    static void playeronly_callBackH(dActor_c *self, dActor_c *other);    ///< @unofficial Empty (4 bytes).
    static void playeronly_callBackW(dActor_c *self, dActor_c *other, u8 dir); ///< @unofficial Empty (4 bytes).
    static bool checkRevHead(dActor_c *self, dActor_c *other);            ///< @unofficial Always false.
    static bool checkRevFoot(dActor_c *self, dActor_c *other);            ///< @unofficial True while the block's @p mSpeed.y is above 0.
    static bool checkRevWall(dActor_c *self, dActor_c *other, u8 dir);    ///< @unofficial Always false.
    static void clear_block_collcallback(dCc_c *self, dCc_c *other);      ///< @unofficial dCc_c hit callback; dispatches on the other actor's kind.

    // ------------------------------------------------------------------
    // Non-static member functions. Classified the same way: here r3 is `this`
    // and the declared parameters start in r4.
    // ------------------------------------------------------------------

    int ObjBgHitCheck();          ///< @unofficial Resolves which face was hit into #m_694 / #m_68d and returns a 0..3 code.
    void ObjBg_PonCheck();        ///< @unofficial
    void ObjBg_PonCheck_jump();   ///< @unofficial Loops over the four players.
    void Block_CreateClearSet(float scale); ///< @unofficial One-shot reset run at creation: stores @p scale in #m_630, loads the scale limits, fills every per-player array with -1 / 0 and sets #m_660 = 8, #m_68a = 2.
    void HopCoinBgcheckSet();     ///< @unofficial Fills #mHopCoinSensor and registers it with @p dActor_c's @p dBc_c as the head sensor.
    bool HopCoinBgcheck();        ///< @unofficial
    void Block_ExecuteClearSet(); ///< @unofficial Per-frame reset of #mHitPlayerF, #mHitPlayerH, #m_674, #m_67f, #m_64c, #m_680 and #m_67d.
    void jumpdai_set();           ///< @unofficial Spawns the jump platform.
    void itemkey_set(u8 kind);    ///< @unofficial
    void item_ivy_set(u8 a, u8 b); ///< @unofficial
    bool isYossyColor(u16 color); ///< @unofficial Owns the file-local static @p l_yoshi_color (.rodata 0x802EE5C0, 0x10 bytes).
    u16 yossy_color_search();     ///< @unofficial
    /// @note `unsigned long`, NOT `u32`. These mangle to `...FUl`; `u32` is
    /// `unsigned int` and mangles `Ui`, naming symbols that do not exist. The
    /// emitted instruction words are identical either way, so only a
    /// callee-symbol-name comparison catches the difference. @unofficial
    void yossy_set(unsigned long dir);
    void multi_yossy_set(unsigned long dir);       ///< @unofficial
    void eggitem_set(unsigned long dir);           ///< @unofficial
    void multi_eggitem_set(unsigned long dir);     ///< @unofficial

    /// @brief Whether the given item spawns as a multiplayer set. @unofficial
    /// @note Returns `u8`, not `bool`: `bool` adds `neg`/`or`/`srwi` and makes
    /// the body 28 bytes instead of 16. Never called inside this TU -- the
    /// RELs call it. 0x80022770.
    u8 isMultiItem(int itemNo);
    void player_set(int mode, int dir); ///< @unofficial Owns the file-local static @p l_player_mode (.rodata 0x802EE5F0, 0x18 bytes).
    void continue_star_check(int *mode, s8 playerNo); ///< @unofficial Rewrites @p *mode in place.
    bool player_bigmario_check(s8 playerNo); ///< @unofficial
    bool propeller_kinoko_check(int mode, s8 playerNo); ///< @unofficial Tail-calls @ref player_bigmario_check.
    int playernumber_set();       ///< @unofficial Number of players in game, minus one (floored at one player).
    bool YoshiEggCreateCheck(int mode); ///< @unofficial
    void item_sound_set(mVec3_c &pos, int mode, s8 playerNo, u8 a, u8 b); ///< @unofficial

    /// @brief 2.0f. Nothing in this TU loads it -- the derived blocks in the
    /// RELs do -- but it is defined here, first in this TU's @p .sdata2 block at
    /// 0x8042B570, immediately before the literal pool. Define it at the top of
    /// d_a_en_blockmain.cpp. @unofficial
    static const float c_YSPD;

    // ------------------------------------------------------------------
    // Members. daEnBlockMain_c's own data begins at 0x524: sizeof(dEn_c) is
    // 0x528 but its data size is 0x524, and MWCC reuses a base class's tail
    // padding, exactly as in daEnBrosBase_c and daEnJimenPakkunBase_c.
    //
    // The upper bound is pinned externally, not guessed: daEnCoinMain_c derives
    // from this class and its own first member is at 0x698 (`addi rN, r3, 0x698`
    // in __dt__14daEnCoinMain_cFv). Combined with 0x694 being the highest offset
    // this TU touches, that fixes the end of the layout.
    // ------------------------------------------------------------------

    /// @brief [0x524] Head sensor handed to @p dActor_c::mBc by @ref HopCoinBgcheckSet.
    /// @details Filled with { 0x80020001, -0x7000, 0x7000, 0x10000 } and passed
    /// as the *head* sensor of @p dBc_c::set(this, NULL, &this, NULL). @unofficial
    sBcSensorLine mHopCoinSensor;

    /// @brief [0x534] The tile-collision registration for the block itself.
    /// @details 0xE4 bytes, so it ends exactly at 0x618. Its flags word is read
    /// as @p 0x60c(this) by the wall callbacks. This is the class's only
    /// destructible member -- the destructor is just this plus @p dEn_c's. @unofficial
    dBg_ctr_c mBgCtr;

    /// @brief [0x618] Motion accumulator used by the three @p _Diff states.
    /// @details Zeroed on entry and then advanced by @p mSpeed each frame
    /// instead of moving the actor; the derived block reads it back. @unofficial
    mVec3_c mMoveDiff;

    u8 m_624[0x630 - 0x624];  ///< [0x624] Never referenced in this TU or in daEnCoinMain_c. @unused

    float m_630;              ///< [0x630] The float argument of @ref Block_CreateClearSet, stored and never read here. @unofficial
    float mMoveYAccel;        ///< [0x634] Per-frame change applied to @p mSpeed.y by the four moving states. 0.281f rising, -2.0f falling. @unofficial
    float mScaleMin;          ///< [0x638] Lower clamp for @ref block_scale_set. @unofficial
    float mScaleStep;         ///< [0x63C] Per-frame scale delta for @ref block_scale_set. @unofficial
    float mScaleMax;          ///< [0x640] Upper clamp for @ref block_scale_set. @unofficial
    int mEndTimer;            ///< [0x644] Set to 6 by @ref initializeState_DownMoveEnd / @ref initializeState_DownMove_DiffEnd; counted down each frame and, at 0, tail-calls the matching @p block_*_end hook. @unofficial
    int m_648;                ///< [0x648] Zeroed by @ref Block_CreateClearSet; driven by the two large unnamed item routines at 0x80022810 / 0x80022B10. @unofficial
    int m_64c;                ///< [0x64C] Set to 1 by the wall callbacks when the "hard hit" flag is on; read by @ref ObjBgHitCheck and cleared every frame. @unofficial

    /// @brief [0x650] Per-player item mode, one @p int each.
    /// @details Written wholesale by the unnamed routine at 0x800221E0, whose
    /// loop is bounded by @ref playernumber_set. @unofficial
    int mItemMode[PLAYER_COUNT];

    u16 m_660;                ///< [0x660] Set to 8 by @ref Block_CreateClearSet; tested against 0 by the foot/head callbacks. Unsigned: all three reads in the TU are `lhz`; `s16` emits `lha` and costs a word. @unofficial
    s16 m_662;                ///< [0x662] Set to 1 by the collider callback at 0x800211B0. @unofficial
    u8 m_664[0x667 - 0x664];  ///< [0x664] @unused
    u8 m_667;                 ///< [0x667] Raised by @ref common_callBackF_enemy for the two enemy types that need the death cry. @unofficial

    /// @brief [0x668] Foot contact: player standing on top of the block, per
    /// player slot, or -1.
    /// @details Written as @p mHitPlayerF[no] = no by @ref callBackF and
    /// @ref nomal_callBackF; reset to -1 every frame by @ref Block_ExecuteClearSet.
    /// (@p F / @p H / @p W follow @p dBg_ctr_c's CallbackF / CallbackH /
    /// CallbackW, which are named from the *other* actor's point of view -- so
    /// "F" is the actor's foot landing on this block, and "H" is its head hitting
    /// this block from below.) @unofficial
    s8 mHitPlayerF[PLAYER_COUNT];

    /// @brief [0x66C] Head contact: player who bumped the block from below, per
    /// player slot, or -1.
    /// @details Written by @ref common_callBackH; also reset every frame. @unofficial
    s8 mHitPlayerH[PLAYER_COUNT];

    /// @brief [0x670] Wall contact: player who hit the block from the side, per
    /// player slot, or -1.
    /// @details Written by @ref callBackW and @ref side_block_moveset. Note it is
    /// **not** cleared by @ref Block_ExecuteClearSet, unlike the other two. @unofficial
    s8 mHitPlayerW[PLAYER_COUNT];

    s8 m_674;                 ///< [0x674] The last side-hit player (not indexed by player). Reset to -1 each frame. @unofficial
    u8 m_675[PLAYER_COUNT];   ///< [0x675] Zeroed by @ref Block_CreateClearSet; read by @ref ObjBg_PonCheck / @ref ObjBg_PonCheck_jump. @unofficial
    u8 m_679[PLAYER_COUNT];   ///< [0x679] Zeroed by @ref Block_CreateClearSet. @unofficial
    u8 m_67d;                 ///< [0x67D] Raised by @ref enemy_only_callBackW. @unofficial
    u8 m_67e;                 ///< [0x67E] The direction that came with the hit that raised #m_67d. @unofficial
    u8 m_67f;                 ///< [0x67F] "Block is active / has been bumped". Gates @ref ObjBgHitCheck and both enemy callbacks; cleared by @ref block_scale_set as the block shrinks back. @unofficial
    /// @brief [0x680] Which contact @ref ObjBgHitCheck should resolve.
    /// @details 1 from @ref common_callBackH, 3 from @ref side_block_moveset,
    /// and a @p daPlBase_c::isStatus result from @ref callBackF.
    /// @ref ObjBgHitCheck reads it as 2 -> scan #mHitPlayerF, 3 -> scan
    /// #mHitPlayerW; anything else falls through to the #mHitPlayerH path
    /// gated by #m_67f / #m_64c. @unofficial
    u8 m_680;
    u8 m_681[PLAYER_COUNT];   ///< [0x681] Per-player side-hit direction + 1, written by @ref callBackW. @unofficial
    u8 m_685[PLAYER_COUNT];   ///< [0x685] Per-player flag written alongside #mHitPlayerF by the two foot callbacks. @unofficial
    u8 m_689;                 ///< [0x689] Non-zero suppresses every hit callback. @unofficial
    u8 m_68a;                 ///< [0x68A] Bit set; initialised to 2, bit 1 ORed in by @ref common_callBackH. @unofficial
    u8 m_68b;                 ///< [0x68B] @unofficial
    u8 m_68c;                 ///< [0x68C] Non-zero suppresses the collider callback at 0x800211B0. @unofficial
    u8 m_68d;                 ///< [0x68D] Hit direction/face code, 2 or 3; also carries @ref side_block_moveset's @p dir. @unofficial
    u8 m_68e[0x690 - 0x68E];  ///< [0x68E] @unused (alignment for #m_690)

    /// @brief [0x690] Block kind. Only ever compared against 2 in this TU
    /// (@ref jumpdai_set, @ref itemkey_set and the two unnamed item routines);
    /// it is written by the derived blocks. @unofficial
    int m_690;

    s8 m_694;                 ///< [0x694] Scratch: the player number @ref ObjBgHitCheck resolved for this frame. @unofficial
    // 3 bytes of trailing padding -> sizeof(daEnBlockMain_c) == 0x698, which is
    // where daEnCoinMain_c's own first member sits. Confirmed by compiling
    // `char probe[sizeof(daEnBlockMain_c)];` and reading the emitted `.skip`.
};
