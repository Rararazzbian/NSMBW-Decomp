#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_wm_bgm_sync.hpp>

/**
* @brief The actor for the killer cannon's fired projectile on the World Map (World 7).
* @unofficial Reconstructed from anonymous (unnamed) target symbols; class name and every
* member name are inferred from codegen evidence and the fProfile::WM_KILLERBULLET slot this
* unit's profile object is placed at, not from any mangled name. Confirmed as daWmKiller_c's
* own spawned child (see wip/wm_units/agent_killer/d_a_wm_killer.cpp's unk_167F20).
*
* Derives DIRECTLY from dWmDemoActor_c (confirmed: ctor calls `__ct__14dWmDemoActor_cFv`
* directly, NOT via dWmObjActor_c -- unlike daWmKiller_c, which goes through dWmObjActor_c).
* sizeof == 0x208 (classInit's `li r3, 0x208` operand).
* @ingroup bases
*/
/// @unofficial Not yet landed anywhere in the project (no header exists) -- forward-declared
/// here, shadow-only, just enough to call the one confirmed member (#calcRotate, confirmed by
/// name from the target's own mangled `calcRotate__12dWmRotater_cFv`).
/// @unofficial Not yet landed anywhere in the project (no header exists -- confirmed by
/// grepping include/) -- forward-declared here, shadow-only. Genuinely polymorphic: dtk
/// reports its vtable object (lbl_2_data_43E34) as 0xc bytes, but the relocations inside it
/// run to at least +0x28 (two null words -- offset-to-top and RTTI -- followed by function
/// pointers is a real vtable; dtk's own reported size is unreliable and the relocations are
/// the authority). Only the one confirmed virtual (the destructor, from the identical
/// slot-2/offset-8 release shape shared with #daWmKillerBullet_c::mBgmSync) is modelled;
/// #calcRotate is called directly (`bl calcRotate__12dWmRotater_cFv`, not a vtable dispatch),
/// so it is NOT virtual.
class dWmRotater_c {
public:
    virtual ~dWmRotater_c();
    void calcRotate();
};

class daWmKillerBullet_c : public dWmDemoActor_c {
public:
    daWmKillerBullet_c(); ///< @copydoc dWmDemoActor_c::dWmDemoActor_c
    ~daWmKillerBullet_c(); ///< @copydoc dWmDemoActor_c::~dWmDemoActor_c

    // Vtable slots confirmed via check_vtable.py against lbl_2_data_455C0:
    // slot 2=create(fn_2_168860), 5=doDelete(fn_2_168C70), 8=execute(fn_2_168AB0),
    // 11=draw(fn_2_168C00), 24=processCutsceneCommand(fn_2_169BC0).
    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    // The 5-entry state-handler table (lbl_2_rodata_89F8), confirmed by the coordinator's own
    // relocation count -- table order is the STATE order, not address order (entries 3/4 are
    // out of address order). Dispatched from #execute via `this->*table[m_1b0]()`.
    void state0(); ///< @unofficial fn_2_168EB0, table entry 0.
    void state1(); ///< @unofficial fn_2_168FF0, table entry 1.
    void state2(); ///< @unofficial fn_2_169280, table entry 2.
    void state3(); ///< @unofficial fn_2_1690F0, table entry 3.
    void state4(); ///< @unofficial fn_2_168F10, table entry 4.

    // Helpers shared by multiple state handlers, all NOT YET AUTHORED this round (still bare
    // distinct stubs) -- signatures/return types inferred from call sites only.
    bool checkParentFlag(); ///< @unofficial fn_2_169500 -- confirmed content: a tail call to
                              ///< WM_KILLER's own `unk_1684A0(false)` on #mParentKiller. A real,
                              ///< cross-unit-confirmed call, not guessed.
    void endEffectAndResetState(); ///< @unofficial fn_2_168E60.
    void endStateOrTransition(); ///< @unofficial fn_2_168F50.
    bool unk_169530(); ///< @unofficial fn_2_169530 -- called from #state1.
    void unk_169550(); ///< @unofficial fn_2_169550 -- the WM_KILLER cross-unit dependency
                         ///< (HANDOFF's `R_2_1_169550`). Cycles all 10 of #mParentKiller's
                         ///< own bullet children then activates the next one round-robin.
    void unk_169430(); ///< @unofficial fn_2_169430, called from #unk_1694A0.
    void unk_168C80(); ///< @unofficial fn_2_168C80, called from #create.
    void unk_168990(); ///< @unofficial fn_2_168990, called from #create.
    void unk_1694A0(); ///< @unofficial fn_2_1694A0.
    void *unk_169510(); ///< @unofficial fn_2_169510 -- returns a pointer used as a float source.
    void unk_1691A0(); ///< @unofficial fn_2_1691A0.
    void unk_1695E0(); ///< @unofficial fn_2_1695E0.
    void unk_1698E0(); ///< @unofficial fn_2_1698E0.
    bool unk_169F00(); ///< @unofficial fn_2_169F00 -- called from #state4, member (implicit
                         ///< `this`), NOT the same function as #checkParentFlag.
    void unk_168D50(); ///< @unofficial fn_2_168D50, called from #execute's own tail.

    // Newly authored this round -- all MATCHED (content confirmed, symbol-name-only residuals).
    dBase_c *unk_1693C0(); ///< @unofficial fn_2_1693C0 -- a dBase_c::searchBaseByProfName
                             ///< (WM_KILLER, ...) loop, matching the caller's own upper-mParam
                             ///< byte against each found actor's own low mParam byte. Scouted
                             ///< as #unk_168990's own case-0 `mParentKiller` finder, but not
                             ///< yet wired into that still-unauthored caller.
    void unk_169080(); ///< @unofficial fn_2_169080 -- transitions to state 3, reloads #m_1b8
                         ///< from the shared table, and fires the same "skl_root"-attached
                         ///< effect #state2 also fires (same #fn_80103520 call shape). Scouted
                         ///< as one of #unk_168990's own callees, but not yet wired in there.
    bool unk_169B80(int delta); ///< @unofficial fn_2_169B80 -- adds \p delta to #m_1c8
                                  ///< (a wrapping 16-bit counter stored in an `int`), wrapping
                                  ///< at 0x10000 and returning whether it wrapped; also mirrors
                                  ///< the low 16 bits into #mAngle's own `z` component. MATCHED.
    bool unk_169DA0(); ///< @unofficial fn_2_169DA0 -- null-checks #mParentKiller, then compares
                         ///< `mPos.distTo(mParentKiller->mPos)` against a shared threshold
                         ///< constant (`R_2_4_89B8[5]`). PARKED, see the .cpp's own note.
    void unk_169E10(); ///< @unofficial fn_2_169E10 -- builds an offset spawn position from
                         ///< #mPos plus two shared-table deltas, calls
                         ///< `_initDemoJumpBase` (real base-class member) with scales derived
                         ///< from #mScale times two more shared-table entries, then saves and
                         ///< restores #mAngle3D around a `setDirection` call whose own dir arg
                         ///< is a cached vector in this unit's own `.bss` (#s_bssDir10, the
                         ///< `+0x10` slot #unk_168990's own case1/case2 branches also reach),
                         ///< and finally clears any active effect (#m_1e4).
    void unk_168F00(); ///< @unofficial fn_2_168F00 -- a true tail call (leaf, no frame): sets
                         ///< #m_1b0 to 4, then falls straight into #unk_169E10 as its own last
                         ///< action.

    // dWmDemoActor_c itself ends at 0x184 (confirmed via the landed daWmPeach_c's own
    // `addi r3, r31, 0x184` for its own first member -- daWmPeach_c also derives directly
    // from dWmDemoActor_c). The target wants this unit's own mAllocator at 0x188, so there is
    // a real 4-byte field here -- matching the same "unused mUnkXXX right after the base class
    // ends" pattern already confirmed on daWmKiller_c (mUnk188) and WM_START.
    u32 mUnk184; ///< @unused @unofficial offset 0x184.
    dHeapAllocator_c mAllocator; ///< The allocator. @unofficial offset 0x188.
    m3d::smdl_c mModel; ///< The model. @unofficial offset 0x1a4. sizeof(m3d::smdl_c) == 0xc,
                          ///< probed directly (vtable ptr + scnLeaf_c::mpScn + bmdl_c::mpAnm,
                          ///< no other fields) -- confirmed NOT the source of the size gap below.

    // @unofficial offset 0x1b0. A state index: #draw and #execute both branch on it, and
    // #execute uses it (times 0xc) to index the 5-entry state-handler table above. NOT a
    // vtable slot itself (confirmed via check_vtable.py).
    int m_1b0;

    // @unofficial offset 0x1b4. Confirmed via #checkParentFlag's own content: a tail call
    // `mParentKiller->unk_1684A0(false)` at the real address 0x1684A0, inside WM_KILLER's own
    // claimed .text range. Real type is `daWmKiller_c*`; kept `void*` here since that class's
    // real header is not shared across agent directories yet -- calls go through a raw
    // extern "C" wrapper matching its real mangled name (see the .cpp).
    void *mParentKiller;

    int m_1b8; ///< @unofficial offset 0x1b8. A decrementing counter/timer (`subi ..., 1`).

    bool m_1bc; ///< @unofficial offset 0x1bc. Set true unconditionally by #create.
    u8 mPad_1bd[0x3]; ///< @unofficial offset 0x1bd, size 0x3 -- placeholder, NOT verified.

    int m_1c0; ///< @unofficial offset 0x1c0. Zeroed by #state2.

    u8 mPad_1c4[0x4]; ///< @unofficial offset 0x1c4, size 0x4 -- placeholder, NOT verified.

    // @unofficial offset 0x1c8. Confirmed `int` (word-width `stw`/`lwz` in #unk_169B80) rather
    // than padding -- a wrapping counter, `+= delta` then wrapped at 0x10000. Splits what had
    // been treated as one 0x10-byte padding run; the remainder below is still unverified.
    int m_1c8;

    u8 mPad_1cc[0x8]; ///< @unofficial offset 0x1cc, size 0x8 -- placeholder, NOT verified.

    bool m_1d4; ///< @unofficial offset 0x1d4. Set/checked by #execute (a "rotation enabled"
                 ///< flag, gating a #calcRotate call through #m_1fc).

    u8 mPad_1d5[0xf]; ///< @unofficial offset 0x1d5, size 0xf -- placeholder, NOT verified.

    int m_1e4; ///< @unofficial offset 0x1e4. An effect ID: -1 == "no active effect" (checked
                ///< `< 0`), passed to `dWmEffectManager_c::endEffect(int)` and reset to -1
                ///< after.
    int m_1e8; ///< @unofficial offset 0x1e8. A second decrementing counter/timer, distinct
                ///< from #m_1b8.

    float m_1ec[3]; ///< @unofficial offset 0x1ec, size 0xc. A target position (mVec3_c-shaped);
                      ///< #state3 measures distance from #m_1ec to `mPos`.

    bool m_1f8; ///< @unofficial offset 0x1f8. Set by #state1.
    bool m_1f9; ///< @unofficial offset 0x1f9. Zeroed unconditionally at the top of #state2.

    u8 mPad_1fa[0x2]; ///< @unofficial offset 0x1fa, size 0x2 -- placeholder, NOT verified.

    // @unofficial offset 0x1fc. Confirmed `dWmRotater_c*` from #execute's own
    // `calcRotate__12dWmRotater_cFv` call -- a real mangled name, not guessed. Also checked
    // and released via a virtual call in the destructor.
    dWmRotater_c *m_1fc;

    // @unofficial offset 0x200. Confirmed `dWmBgmSync_c*` -- include/game/bases/d_wm_bgm_sync.hpp
    // is a real, already-landed header (found by grepping include/ before shadow-declaring it,
    // per the coordinator's own precedent on agent_board). Heap-allocated in #create(),
    // released via an ordinary `delete` in the destructor -- same pattern as agent_board's own
    // mBgmSync.
    dWmBgmSync_c *mBgmSync;

    bool m_204; ///< @unofficial offset 0x204. Set/checked by #state1/#endStateOrTransition.
    bool m_205; ///< @unofficial offset 0x205. Checked by #execute (distinct byte from #m_204,
                 ///< confirmed from separate `lbz` offsets 0x204/0x205).

    u8 mPad_206[0x2]; ///< @unofficial offset 0x206, size 0x2 -- placeholder padding up to
                        ///< sizeof==0x208, NOT individually verified.
};

