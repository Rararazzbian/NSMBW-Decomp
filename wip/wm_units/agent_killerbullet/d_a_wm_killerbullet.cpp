#include <game/bases/d_a_wm_killerbullet.hpp>
#include <game/bases/d_cs_seq_manager.hpp>

ACTOR_PROFILE(WM_KILLERBULLET, daWmKillerBullet_c, 0);

daWmKillerBullet_c::daWmKillerBullet_c() : m_1d4(false) {}

// NOT YET AUTHORED (0x104 bytes). Manually destroys #mModel/#mAllocator then (conditionally,
// guarded on the ORIGINAL `this` pointer, not a count -- the "delete this" complete-object
// destructor pattern) chains into dWmDemoActor_c's own embedded mModel/mHeapAllocator members
// and dWmActor_c's dtor, matching the shape already confirmed on the landed corpus. The one
// piece NOT yet characterised: two virtual "release" calls on #m_1fc/#m_200 (vtable slot 2,
// argument true) before any of that -- their real class is unconfirmed, so left unauthored
// rather than guessed via a raw vtable-pointer cast.
daWmKillerBullet_c::~daWmKillerBullet_c() {}

// create(). Vtable slot 2, confirmed via check_vtable.py. NOT YET AUTHORED (0x128 bytes) --
// content not yet read.
int daWmKillerBullet_c::create() {
    m_1b0 = 1;
    return SUCCEEDED;
}

// execute(). Vtable slot 8, confirmed via check_vtable.py. FULLY CHARACTERISED, NOT YET
// AUTHORED (0xdc bytes): a virtual dispatch through #m_200 (vtable slot 3, no owned type yet),
// the same processCutsceneCommand-via-secondary-vtable idiom already landed on daWmKiller_c's
// own execute(), a state check on #m_1b0 against a POINTER-TO-MEMBER-FUNCTION TABLE
// (lbl_2_rodata_89F8, `this->*table[m_1b0]()` via `__ptmf_scall`) that strongly implies a
// state-machine architecture where several of this unit's other functions are per-state
// handlers, a conditional calcRotate() through #m_1fc gated on #m_1d4, then CalcShadow with
// two float constants and a call into fn_2_168D50 (not yet named).
int daWmKillerBullet_c::execute() {
    m_1b0 = 2;
    return SUCCEEDED;
}

// draw(). Vtable slot 11, confirmed via check_vtable.py. Confirmed content: draws (mModel's
// own entry() virtual, then DrawShadow(true)) when EITHER m_1b0 is neither 0 nor 1, OR
// ACTOR_PARAM(Kind)-shaped (mParam>>16)==1 (the same upper-half-of-mParam field convention
// already established on daWmKiller_c).
int daWmKillerBullet_c::draw() {
    if ((m_1b0 != 0 && m_1b0 != 1) || (int) (mParam >> 16) == 1) {
        mModel.entry();
        DrawShadow(true);
    }
    return SUCCEEDED;
}

// doDelete(). Vtable slot 5, confirmed via check_vtable.py. Trivial -- matches the target's
// own `li r3, 1; blr` exactly (2 bytes).
int daWmKillerBullet_c::doDelete() {
    return SUCCEEDED;
}
