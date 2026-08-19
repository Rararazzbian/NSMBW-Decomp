#include <game/bases/d_a_wm_killerbullet.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_effect_manager.hpp>

ACTOR_PROFILE(WM_KILLERBULLET, daWmKillerBullet_c, 0);

// #checkParentFlag's target -- WM_KILLER's own unk_1684A0(bool), a real, already-landed-in-
// draft function at 0x1684A0 (inside daWmKiller_c's own claimed .text range). Declared via its
// exact mangled name so the linker resolves it directly once both units exist -- a landing-
// order dependency the coordinator has recorded, not something authored here.
extern "C" bool unk_1684A0__12daWmKiller_cFb(void *self, bool arg);

// #execute's CalcShadow float constants and the state-handler table live in this unit's own
// .data/.rodata (lbl_2_data_45428, lbl_2_rodata_89F8) -- not yet named/declared here since the
// section bounds work is still open this round.

daWmKillerBullet_c::daWmKillerBullet_c() : m_1d4(false) {}

// NOT YET AUTHORED (0x104 bytes). See the header's own note on #m_1fc/#m_200's destructor
// release calls -- their real class is unconfirmed for #m_200, so left unauthored rather than
// guessed via a raw vtable-pointer cast.
daWmKillerBullet_c::~daWmKillerBullet_c() {}

// create(). Vtable slot 2, confirmed via check_vtable.py. NOT YET AUTHORED (0x128 bytes) --
// content not yet read.
int daWmKillerBullet_c::create() {
    m_1c0 = 1;
    return SUCCEEDED;
}

// execute(). Vtable slot 8, confirmed via check_vtable.py. Fully decoded from the target:
// a virtual dispatch through #m_200 (vtable slot 3, real class unconfirmed -- called via a raw
// vtable-pointer walk rather than an invented type, matching the destructor's own precedent),
// the same processCutsceneCommand-via-secondary-vtable idiom already landed on daWmKiller_c's
// own execute(), then a state check gating the 5-entry state-handler table dispatch
// (`this->*sc_StateTable[m_1b0]()`), a conditional calcRotate() through #m_1fc gated on
// #m_1d4, then CalcShadow with two float constants and a call into fn_2_168D50 (not yet named).
typedef void (daWmKillerBullet_c::*StateFunc_t)();
static const StateFunc_t sc_StateTable[5] = {
    &daWmKillerBullet_c::state0,
    &daWmKillerBullet_c::state1,
    &daWmKillerBullet_c::state2,
    &daWmKillerBullet_c::state3,
    &daWmKillerBullet_c::state4,
};

int daWmKillerBullet_c::execute() {
    // m_200's own vtable slot 3 (offset 0xc), no extra args -- real class unconfirmed, raw
    // dispatch rather than an invented type (see this function's own note above).
    void **m200Vtbl = *(void ***) m_200;
    ((void (*)(void *)) m200Vtbl[3])(m_200);

    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    if (csSeqMng->FUN_80915600()) {
        processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    }

    if (m_1b0 != 3) {
        bool skipDispatch = false;
        if (csSeqMng->FUN_80915600() && csSeqMng->GetCutName() == 0x38) {
            // fall through to the GetCutName()==0x4d/0x4c checks
        } else if (!m_205 && csSeqMng->FUN_80915600()) {
            skipDispatch = true;
        }
        if (!skipDispatch && csSeqMng->GetCutName() != 0x4d && csSeqMng->GetCutName() != 0x4c) {
            (this->*sc_StateTable[m_1b0])();
        }
    }

    if (m_1d4 && csSeqMng->GetCutName() != 0x56) {
        m_1fc->calcRotate();
    }

    CalcShadow(0.8f, 0.7f); // lbl_2_data_45428+0x10/+0x14, real values -- but stored as
                             // fields of a larger DATA struct, not a rodata literal pool;
                             // that struct is not yet declared (open .data-bounds work).
    unk_168D50();
    return SUCCEEDED;
}

// draw(). Vtable slot 11, confirmed via check_vtable.py. MATCHES.
int daWmKillerBullet_c::draw() {
    if ((m_1b0 != 0 && m_1b0 != 1) || (int) (mParam >> 16) == 1) {
        mModel.entry();
        DrawShadow(true);
    }
    return SUCCEEDED;
}

// doDelete(). Vtable slot 5, confirmed via check_vtable.py. MATCHES (trivial).
int daWmKillerBullet_c::doDelete() {
    return SUCCEEDED;
}

// #checkParentFlag(). Confirmed content: a tail call to WM_KILLER's own unk_1684A0(false) on
// #mParentKiller -- a cross-unit-confirmed call, not guessed (see the extern declaration above
// and its own note on the landing-order dependency this creates).
bool daWmKillerBullet_c::checkParentFlag() {
    return unk_1684A0__12daWmKiller_cFb(mParentKiller, false);
}

// state0 (table entry 0, fn_2_168EB0). Confirmed content: only acts when the low ACTOR_PARAM
// half of mParam is zero-shifted (i.e. (mParam>>16)==0, checked via srwi.) AND #checkParentFlag()
// is true.
void daWmKillerBullet_c::state0() {
    if ((int) (mParam >> 16) == 0 && checkParentFlag()) {
        endStateOrTransition();
    }
}

// state4 (table entry 4, fn_2_168F10). Confirmed content: calls fn_2_169F00 (not yet named --
// NOT the same as #checkParentFlag) and, if it returns true, ends the state.
void daWmKillerBullet_c::state4() {
    if (unk_169F00()) {
        endStateOrTransition();
    }
}


// NOT YET AUTHORED helpers (distinct scratch stubs, avoiding the bool-collapses-to-1 trap by
// writing into a real, in-bounds, non-bool scratch field -- m_1c0).
void daWmKillerBullet_c::endEffectAndResetState() { m_1c0 = 2; }
void daWmKillerBullet_c::endStateOrTransition() { m_1c0 = 3; }
void daWmKillerBullet_c::unk_1694A0() { m_1c0 = 4; }
bool daWmKillerBullet_c::unk_169530() { m_1c0 = 14; return false; }
void *daWmKillerBullet_c::unk_169510() { m_1c0 = 5; return nullptr; }
void daWmKillerBullet_c::unk_1691A0() { m_1c0 = 6; }
void daWmKillerBullet_c::unk_1695E0() { m_1c0 = 7; }
void daWmKillerBullet_c::unk_1698E0() { m_1c0 = 8; }
bool daWmKillerBullet_c::unk_169F00() { m_1c0 = 9; return false; }
void daWmKillerBullet_c::unk_168D50() { m_1c0 = 10; }

// state1/state2/state3 (table entries 1/2/3) -- fully decoded from the target, NOT YET
// AUTHORED this round (helper bodies above are still bare stubs, so authoring these three now
// would not verify cleanly). Left as distinct placeholders.
// state1 (table entry 1, fn_2_168FF0). Confirmed content: if #checkParentFlag() is false,
// clears #m_204 and calls #endEffectAndResetState(); otherwise, a not-yet-named bool check
// (fn_2_169530) may set #m_1f8, and if #m_1f8 is set, either decrements #m_1b8 (a cooldown-
// shaped counter) or, once it reaches zero, calls #unk_1691A0().
void daWmKillerBullet_c::state1() {
    if (!checkParentFlag()) {
        m_204 = false;
        endEffectAndResetState();
    } else {
        if (unk_169530()) {
            m_1f8 = true;
        }
        if (m_1f8) {
            if (m_1b8 > 0) {
                m_1b8 -= 1;
            } else {
                unk_1691A0();
            }
        }
    }
}
void daWmKillerBullet_c::state2() { m_1c0 = 12; }
void daWmKillerBullet_c::state3() { m_1c0 = 13; }
