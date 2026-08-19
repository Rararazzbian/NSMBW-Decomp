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
class daWmKillerBullet_c : public dWmDemoActor_c {
public:
    daWmKillerBullet_c(); ///< @copydoc dWmDemoActor_c::dWmDemoActor_c
    ~daWmKillerBullet_c(); ///< @copydoc dWmDemoActor_c::~dWmDemoActor_c

    // Vtable slots confirmed via check_vtable.py against lbl_2_data_455C0:
    // slot 2=create(fn_2_168860), 5=doDelete(fn_2_168C70), 8=execute(fn_2_168AB0),
    // 11=draw(fn_2_168C00), 24=processCutsceneCommand(fn_2_169BC0).
    virtual int create();
    virtual int execute(); ///< PARTIALLY DECODED (0xdc bytes) -- see #execute's own note. NOT
                             ///< YET AUTHORED -- content well-characterised, not guessed.
    virtual int draw();
    virtual int doDelete();

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
    // #execute uses it (times 0xc) to index a pointer-to-member-function table
    // (lbl_2_rodata_89F8, called via `__ptmf_scall`) -- strongly suggests a state-machine
    // architecture where several of this unit's still-unauthored functions are the individual
    // per-state handlers. NOT a vtable slot itself (confirmed via check_vtable.py).
    int m_1b0;
    // @unofficial offset 0x1b4, size 0x20. Real field(s) not yet identified -- placeholder
    // padding up to the flag byte at 0x1d4, NOT individually verified.
    u8 mPad_1b4[0x20];
    bool m_1d4; ///< @unofficial offset 0x1d4. Set/checked by #execute (a "rotation enabled"
                 ///< flag, gating a #calcRotate call through #m_1fc).

    // @unofficial offset 0x1d5, size 0x27. Real field(s) not yet identified -- placeholder
    // padding, NOT individually verified.
    u8 mPad_1d5[0x27];

    // @unofficial offset 0x1fc. A pointer (type unconfirmed, provisionally void*) checked and
    // released via a virtual call in the destructor, and used by #execute to call a
    // `calcRotate()` member -- likely a dWmRotater_c* or an object wrapping one.
    void *m_1fc;

    // @unofficial offset 0x200. A pointer (type unconfirmed, provisionally void*) with the
    // same destructor-release shape as #m_1fc, and dispatched through its own vtable slot 3
    // (offset 0xc) by #execute. Distinct object -- not the same field.
    void *m_200;

    // @unofficial offset 0x204, size 0x4. Not yet identified -- placeholder padding up to
    // sizeof==0x208 (classInit's own `li r3, 0x208`).
    u8 mPad_204[0x4];
};
