#pragma once
#include <game/mLib/m_vec.hpp>
#include <game/sLib/s_State.hpp>

/// @brief Manages movement of an actor along a line/rail-shaped path (straight
/// segments, 30/45/60-degree diagonals, and circular arcs of several radii).
/// @details Has NO vtable of its own -- confirmed by the absence of
/// `__vt__10dLineMng_c` in the symbol map, and by its constructor never calling
/// a base-class constructor before `__construct_array`. It is a plain,
/// non-polymorphic value member embedded in some other (not yet decompiled)
/// owner class -- likely a background-control actor, going by the neighbouring
/// `dBg_ctr_c` work in this checkout's recent history.
/// @unofficial Reconstructed from the constructor, init(), init_term_ck_pos()
/// and move(); no allocation site was found in wiimj2d, so this is the class's
/// OWN layout only -- the owning class's member name/offset for it is unknown.
/// @ingroup bases
class dLineMng_c {
    // MERGE-LOCAL, NOT IN THE SHARED HEADER -- proposed for lead review, see
    // MERGE.md "local-header overrides not yet folded in".
    //
    // Three unnamed file-scope helpers write dLineMng_c's private mPos/
    // mUnitBasePos directly through a passed-in pointer, so they need friend
    // access:
    //   fn_800C3B20/fn_800C3B60 (author_core): clamp mPos.x/mPos.y against
    //     mUnitBasePos's unit cell. Evidence: both write this->mPos through
    //     the incoming pointer with no accessor calls.
    //   fn_800C1EE0 (author_geom): shared tail of width_cross_chk's
    //     line_cross_chk1 call; writes this->mPos and this->mUnitBasePos.
    //     Evidence: width_cross_chk passes its OWN incoming `this` straight
    //     through unmodified (no `mr r3,...` before the `bl`), and the
    //     target's disassembly shows NO mangled name for the callee at all --
    //     unlike every named `static` member in this unit, which does carry
    //     one -- so it cannot be a class member.
    friend void fn_800C3B20(dLineMng_c *self);
    friend void fn_800C3B60(dLineMng_c *self);
    friend bool fn_800C1EE0(dLineMng_c *, f32, f32, const mVec2_c &, const mVec2_c &, const mVec2_c &, const mVec2_c &);
    // fn_800C31C0 (wip/fix_bighelper): reads self->mPos/self->mOldPos
    // directly through the incoming pointer (no accessor calls in its
    // disassembly, same evidence shape as the three above). It does NOT
    // write any member -- see RESULT.md, this corrects MAPPING.md's "first
    // writer of the 0x58 field" claim, which does not hold for this function.
    friend void fn_800C31C0(dLineMng_c *self);

public:
    dLineMng_c();
    ~dLineMng_c() {}
    // No destructor: `__dt__10dLineMng_cFv` does not exist in the symbol map,
    // so nothing in wiimj2d ever destroys a dLineMng_c. Do NOT declare one --
    // an unused implicit destructor is never emitted anyway, but a declared one
    // risks a linkage mismatch if it ever WOULD be emitted.

    /// @brief Sets up the manager at a starting position and enters StateID_Idle.
    /// @details Zeroes the first 3 slots of mDirVec, lazily fills a static
    /// direction-vector table on first-ever call (guarded by a file-scope
    /// bool), and copies it into the remaining 4 slots. Snaps @p pos to the
    /// UNIT_SIZE grid into mUnitBasePos. Calls `mStateMgr.changeState(StateID_Idle)`
    /// itself -- the constructor only ever passes `sStateID::null`, confirmed
    /// by both `addi rN, r30, null__8sStateID@l` calls in the ctor's
    /// disassembly.
    void init(const mVec2_c &pos, f32 speed, int lineType, u8 param);

    void move(); ///< Per-frame driver. Conditionally runs `mStateMgr.executeState()`, then syncs mOldPos from mPos.

    mVec2_c GetPos() const; ///< @unofficial Returns mPos by value.
    void SetPos(const mVec2_c &pos); ///< @unofficial Writes mPos.

    f32 CalcAdjustPosY(f32, f32); ///< @unofficial f32, argued from dead-code-elimination behaviour. NO caller exists anywhere in this TU, so this could NOT be proven by the call-site method -- weaker evidence than every other return type here. Flagged.
    void SetBaseSpeed(f32 speed); ///< @unofficial Writes mBaseSpeed, negating it first if mReverse is set.
    /// @unofficial Return type changed from `void` to `u16` (gapA/d4_misc):
    /// target.txt shows a `beqlr` (conditional RETURN) right after the value
    /// is computed into r3 and masked to 16 bits -- a void function has no
    /// reason to keep a value alive in r3 across a return point, and a bare
    /// unused arithmetic expression would be dead-code-eliminated entirely
    /// under -O4, not computed and masked on both paths. u16 matches mAngle's
    /// own field type exactly.
    u16 acm_angle() const;

    void start_line_move();
    static bool is_unit_circle2x2(ulong unitID); ///< Confirmed bool: `li r3, 0x1/0x0; blr`.
    static bool is_unit_circle4x4(ulong unitID); ///< Confirmed bool: `li r3, 0x1/0x0; blr`.
    void change_dir(); ///< Negates mBaseSpeed and flips mReverse.
    static u32 getLineUnitNo(f32, f32); ///< @unofficial PROVEN non-void (every mov_to_*/mov_frm_* caller reads r3 right after the `bl`) AND now PROVEN static: THREE independent call sites (check_term, start_line_move, and fn_800C31C0) show no `mr r3,<this>` before the `bl` -- r3 is dead/garbage at every one of those call points. A non-static declaration would force the compiler to emit that mr unconditionally, which the target never has.
    void init_term_ck_pos();
    bool check_term(); ///< PROVEN bool, found independently by two agents: 12 call sites all consume the return value.

    static bool line_cross_slope_check(const mVec2_c &, const mVec2_c &, f32 &, f32 &);
    static bool line_cross_range_check(f32, f32, f32);
    static bool line_cross_chk1(f32, f32, const mVec2_c &, mVec2_c, mVec2_c, mVec2_c &);
    static bool line_cross_chk2(f32, const mVec2_c &, mVec2_c, mVec2_c, f32 &);
    static bool line_cross_chk3(f32, const mVec2_c &, const mVec2_c &);

    bool height_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool width_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    // PROVEN bool, not void: fn_800C31C0 is the first real caller of these 14
    // (the family was previously only ever DECLARED, never called anywhere in
    // this TU). Every call site does `cmpwi r3,0x0` immediately after the
    // `bl` and branches on it -- a void function has no defined r3 to branch
    // on, so these must return a value. See RESULT.md (wip/fix_bighelper).
    bool line0_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool line1_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool line3h_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool line3v_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool line4_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool line5_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool line7_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool line8_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool line9_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool lineA_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool lineB_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool lineC_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool lineD_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool lineE_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
    bool lineF_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);

    bool circle_ul2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
    bool circle_ur2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
    bool circle_dl2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
    bool circle_dr2_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
    bool lineRHUR_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
    bool lineRHUL_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
    bool lineRHLL_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);
    bool lineRHLR_cross_chk(const mVec2_c &, mVec2_c, mVec2_c);

    void circle_nextpos_set(const mVec2_c &, f32);
    void calc_rotate_to_circle_rev(u16, bool);
    void calc_rotate_to_circle_prev(u16, bool);
    bool mov_to_rightupper(ulong, const mVec2_c &, bool);
    bool mov_to_rightlower(ulong, const mVec2_c &, bool);
    bool mov_to_leftupper(ulong, const mVec2_c &, bool);
    bool mov_to_leftlower(ulong, const mVec2_c &, bool);
    void mov_frm_rightupper(const mVec2_c &, bool);
    void mov_frm_leftlower(const mVec2_c &, bool);
    void mov_frm_rightlower(const mVec2_c &, bool);
    void mov_frm_leftupper(const mVec2_c &, bool);
    void move_on_circle_speedset(f32, f32);
    void move_on_circle1(f32, f32);
    void move_on_circle2(f32, f32);
    void move_on_circle3(f32, f32);
    void move_on_circle4(f32, f32);

    // MERGE-LOCAL, NOT IN THE SHARED HEADER -- proposed for lead review, see
    // MERGE.md. Needed by init()/check_term()/start_line_move() (all
    // author_core). Value 16.0f read directly from original/wiimj2d.dol at
    // .sdata2:0x8042CB18 (raw bytes 41 80 00 00). Deliberately left WITHOUT a
    // definition in this TU: giving it an initializer here lets -O4 fold
    // init()'s `pos.x / smc_UNIT_SIZE_X` into a multiply-by-reciprocal
    // (measured: zero `fdivs` emitted vs the target's two) -- so its real
    // defining TU must be elsewhere, not yet decompiled.
    static const float smc_UNIT_SIZE_X;

private:
    // --- The 25 states, in .bss/.text declaration order (verified: both the
    // .bss StateID object addresses and the .text initializeState/finalizeState/
    // executeState triples appear in this exact sequence). None of them are
    // virtual: their `.data` PMF triples are `{ -1, fn_addr, 0 }` (the
    // NON-VIRTUAL encoding; confirmed by using the plain sFStateID_c<T>, not
    // sFStateVirtualID_c<T> -- `__vt__25sFStateID_c<10dLineMng_c>` size 0x34
    // is sFStateID_c's OWN vtable for isSameName/init/exec/finalizeState, not
    // a per-state virtual table). So every state below uses the plain macro.
    STATE_FUNC_DECLARE(dLineMng_c, Idle);
    STATE_FUNC_DECLARE(dLineMng_c, FallDown);
    STATE_FUNC_DECLARE(dLineMng_c, Left45);
    STATE_FUNC_DECLARE(dLineMng_c, Right45);
    STATE_FUNC_DECLARE(dLineMng_c, Side);
    STATE_FUNC_DECLARE(dLineMng_c, Height);
    STATE_FUNC_DECLARE(dLineMng_c, CornerHeightLine);
    STATE_FUNC_DECLARE(dLineMng_c, CornerSideLine);
    STATE_FUNC_DECLARE(dLineMng_c, Left30Left);
    STATE_FUNC_DECLARE(dLineMng_c, Left30Right);
    STATE_FUNC_DECLARE(dLineMng_c, Right30Left);
    STATE_FUNC_DECLARE(dLineMng_c, Right30Right);
    STATE_FUNC_DECLARE(dLineMng_c, Left60Up);
    STATE_FUNC_DECLARE(dLineMng_c, Left60Down);
    STATE_FUNC_DECLARE(dLineMng_c, Right60Down);
    STATE_FUNC_DECLARE(dLineMng_c, Right60Up);
    STATE_FUNC_DECLARE(dLineMng_c, Circle);
    STATE_FUNC_DECLARE(dLineMng_c, Circle2x2Leftup);
    STATE_FUNC_DECLARE(dLineMng_c, Circle2x2Rightup);
    STATE_FUNC_DECLARE(dLineMng_c, Circle2x2LeftDown);
    STATE_FUNC_DECLARE(dLineMng_c, Circle2x2RightDown);
    STATE_FUNC_DECLARE(dLineMng_c, Circle4x4Rightup);
    STATE_FUNC_DECLARE(dLineMng_c, Circle4x4LeftUp);
    STATE_FUNC_DECLARE(dLineMng_c, Circle4x4LeftDown);
    STATE_FUNC_DECLARE(dLineMng_c, Circle4x4RightDown);

    /// @brief Direction/step unit-vectors, indexed by a line-direction id.
    /// @details Confirmed by init_term_ck_pos(): slots [0..2] are zeroed every
    /// call; slots [3..6] are copied from a function-local `static` 8-float
    /// table that is itself lazily filled (guarded by a hidden bool) the FIRST
    /// time init_term_ck_pos() ever runs. @unofficial name and exact values.
    mVec2_c mDirVec[7]; ///< 0x00

    /// @unofficial Zeroed in init(); read/written by nearly every state's
    /// execute method. Almost certainly a per-frame velocity/delta.
    mVec2_c mSpeed; ///< 0x38

    mVec2_c mPos; ///< 0x40 -- GetPos()/SetPos(), confirmed.

    /// @unofficial Set to a copy of the initial pos in init(); overwritten
    /// from mPos every move() call AFTER the state's executeState() runs. So
    /// this is `move()`'s previous-frame position snapshot.
    mVec2_c mOldPos; ///< 0x48

    /// @unofficial UNIT_SIZE-grid-snapped base position, computed once in
    /// init() via `(f32)(int)(pos / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X` on
    /// both axes. Heaviest-used mVec2_c in the whole unit (55-56 refs across
    /// the state execute functions).
    mVec2_c mUnitBasePos; ///< 0x50

    /// @unofficial NOT touched by init() at all -- confirmed no store to
    /// 0x58/0x5c anywhere in init()'s disassembly. Set later, seen written in
    /// the large unnamed fn_800C31C0 (0x894 bytes) and read in
    /// circle_nextpos_set / two more unnamed helpers (fn_800C3B20, fn_800C3B60).
    /// OPEN QUESTION: name and owning function not yet determined.
    mVec2_c mUnk58; ///< 0x58

    f32 mBaseSpeed; ///< 0x60 -- SetBaseSpeed()/change_dir(), confirmed.

    /// @unofficial Zeroed in init(). acm_angle() reads it and adds/subtracts
    /// 0x4000 depending on mReverse, then masks to 16 bits.
    u16 mAngle; ///< 0x64

    u8 mType; ///< 0x66 -- @unofficial the `u8 param` argument of init(), stored verbatim.

    /// @unofficial Zeroed at the very END of init() (after the
    /// changeState(StateID_Idle) virtual call returns). move() skips its
    /// `mStateMgr.executeState()` call when this is NONZERO -- so this reads
    /// as a "don't run the state machine this frame" gate, not an "is
    /// initialized" flag. Name and exact semantics unconfirmed.
    u8 mUnk67; ///< 0x67

    /// @unofficial Direction/reverse flag. change_dir() flips it and negates
    /// mBaseSpeed together; acm_angle() and SetBaseSpeed() both branch on it.
    u8 mReverse; ///< 0x68

    /// @unofficial Low byte of init()'s `int lineType` parameter, stored
    /// BEFORE init_term_ck_pos() is called (so init_term_ck_pos() or a state
    /// can read it during setup).
    u8 mLineType; ///< 0x69

    /// @unofficial 0x6a -- a REAL one-byte field, not padding. Two functions
    /// write it through the OBJECT pointer (not r1, so not a stack spill):
    ///   start_line_move()  stb r30, 0x6a(r29)  with r30 = 0, unconditionally
    ///   check_term()       stb r0,  0x6a(r28)  with r0  = 1, on the found path
    /// The earlier "padding" reading was inferred from init() never touching it
    /// -- an argument from silence, and wrong. Only 0x6b is true alignment
    /// padding for mStateMgr's vtable pointer.
    u8 mUnk6a;

    /// @brief The nested state-of-states manager.
    /// @details This is the single biggest structural finding of this unit:
    /// dLineMng_c does NOT use a plain `sFStateMgr_c<dLineMng_c,
    /// sStateMethodUsr_FI_c>` the way Pausewindow_c does. It uses the NESTED
    /// `sFStateStateMgr_c<dLineMng_c, sStateMethodUsr_FI_c,
    /// sStateMethodUsr_FI_c>`, which itself contains a `mainMgr` AND a
    /// `subMgr` (both `sFStateMgr_c<dLineMng_c, sStateMethodUsr_FI_c>`, since
    /// Method1==Method2 here) plus a `currentMgr` pointer -- confirmed by the
    /// symbol map carrying BOTH `sFStateStateMgr_c<...>`/`sStateStateMgr_c<...>`
    /// AND `sFStateMgr_c<...>`/`sStateMgr_c<...>` template instantiations for
    /// this class, and by the constructor writing vtables and running
    /// sStateMethodUsr_FI_c's real (non-inlined) constructor TWICE at two
    /// different sub-offsets (this+0x70 and this+0xac) before writing
    /// `currentMgr = &mainMgr` at this+0xe8.
    ///
    /// Confirmed sub-layout (all offsets relative to dLineMng_c, i.e. +0x6c):
    ///   0x6c: sStateStateMgr_c vtable ptr (overwritten with sFStateStateMgr_c's
    ///         own vtable at the very end of the ctor -- the usual
    ///         base-vtable-then-derived-vtable construction fixup)
    ///   0x70: mainMgr (sFStateMgr_c<dLineMng_c,sStateMethodUsr_FI_c>), size 0x3C
    ///     0x70: vtable ptr, 0x74: mCheck (sStateIDChk_c, has own vtable),
    ///     0x78: mFactory (sFStateFct_c<dLineMng_c>, 0x10 bytes: own vtable at
    ///           +0, a NESTED sFState_c<dLineMng_c> vtable at +4, owner ptr at
    ///           +8, one zeroed word at +0xc), 0x88: mMethod
    ///           (sStateMethodUsr_FI_c, 0x24 bytes, constructed via a REAL
    ///           out-of-line `bl`, not inlined)
    ///   0xac: subMgr, same shape as mainMgr, ends 0xe8
    ///   0xe8: currentMgr (sStateMgrIf_c*), initialised to &mainMgr
    ///   -> sizeof(mStateMgr) == 0x80, dLineMng_c ends at 0xec.
    ///
    /// CONSTRUCTOR PASSES `sStateID::null`, NOT `StateID_Idle`, to this
    /// member's initialiser -- confirmed twice in the ctor disassembly
    /// (`addi rN, r30, null__8sStateID@l` before EACH of the two
    /// sStateMethodUsr_FI_c construction calls). The real first state is set
    /// later, in init(), via a virtual call through the vtable at slot offset
    /// 0x18 (== changeState, by the `(offset-8)/4` slot rule) with
    /// `&StateID_Idle` as the argument.
    sFStateStateMgr_c<dLineMng_c, sStateMethodUsr_FI_c, sStateMethodUsr_FI_c> mStateMgr; ///< 0x6c, size 0x80

    /// @cond
    void dummy() {
        mStateMgr.initializeState();
        mStateMgr.finalizeState();
        mStateMgr.isSubState();
        mStateMgr.returnState();
        mStateMgr.getOldStateID();
        mStateMgr.refreshState();
    }
    /// @endcond

    // sizeof(dLineMng_c) == 0xEC by this evidence. No allocation site (global
    // instance, array, or owning-class member) was found anywhere in
    // wiimj2d_symbols.txt to cross-check this with a STATIC_ASSERT against a
    // known total -- flagged as the one unconfirmed structural fact.
};
