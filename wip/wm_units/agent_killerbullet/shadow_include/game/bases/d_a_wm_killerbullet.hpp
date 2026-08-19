#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_demo_actor.hpp>

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
class dWmRotater_c {
public:
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
    void unk_169430(); ///< @unofficial fn_2_169430, called from #unk_1694A0.
    void unk_1694A0(); ///< @unofficial fn_2_1694A0.
    void *unk_169510(); ///< @unofficial fn_2_169510 -- returns a pointer used as a float source.
    void unk_1691A0(); ///< @unofficial fn_2_1691A0.
    void unk_1695E0(); ///< @unofficial fn_2_1695E0.
    void unk_1698E0(); ///< @unofficial fn_2_1698E0.
    bool unk_169F00(); ///< @unofficial fn_2_169F00 -- called from #state4, member (implicit
                         ///< `this`), NOT the same function as #checkParentFlag.
    void unk_168D50(); ///< @unofficial fn_2_168D50, called from #execute's own tail.

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

    u8 mPad_1bc[0x4]; ///< @unofficial offset 0x1bc, size 0x4 -- placeholder, NOT verified.

    int m_1c0; ///< @unofficial offset 0x1c0. Zeroed by #state2.

    u8 mPad_1c4[0x10]; ///< @unofficial offset 0x1c4, size 0x10 -- placeholder, NOT verified.

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

    // @unofficial offset 0x200. A pointer (type unconfirmed, provisionally void*) with the
    // same destructor-release shape as #m_1fc, and dispatched through its own vtable slot 3
    // (offset 0xc) by #execute, and read at offset 0xd (a byte field) by #state2. Distinct
    // object -- not the same field.
    void *m_200;

    bool m_204; ///< @unofficial offset 0x204. Set/checked by #state1/#endStateOrTransition.
    bool m_205; ///< @unofficial offset 0x205. Checked by #execute (distinct byte from #m_204,
                 ///< confirmed from separate `lbz` offsets 0x204/0x205).

    u8 mPad_206[0x2]; ///< @unofficial offset 0x206, size 0x2 -- placeholder padding up to
                        ///< sizeof==0x208, NOT individually verified.
};

