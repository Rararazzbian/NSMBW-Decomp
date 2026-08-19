#include <game/bases/d_a_wm_killer.hpp>

ACTOR_PROFILE(WM_KILLER, daWmKiller_c, 0);

daWmKiller_c::daWmKiller_c() : m_208(false) {}
daWmKiller_c::~daWmKiller_c() {}

// create(). Confirmed call sequence and the mClipSphere.set(mPos, 100.0f) quad (matching
// castle/WM_START's own create() shape) -- the five sub-calls' own bodies are not yet authored.
int daWmKiller_c::create() {
    unk_167D30();
    unk_167F20();
    mClipSphere.set(mPos, 100.0f);
    unk_167C70();
    unk_167FB0();
    return SUCCEEDED;
}

// NOT YET AUTHORED (0x124 bytes). Vtable slot 8, confirmed via check_vtable.py. Reads the
// far .bss symbol lbl_2_bss_FE40 (see this task's report) -- declare that extern when authoring.
int daWmKiller_c::execute() {
    mPad_1ec[0] = 0x1;
    return SUCCEEDED;
}

int daWmKiller_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// NOT YET AUTHORED (0xa8 bytes). Confirmed NOT a vtable slot (check_vtable.py) -- an ordinary
// member function, called once from #create.
void daWmKiller_c::unk_167C70() {
    mPad_1ec[0] = 0x2;
}

int daWmKiller_c::doDelete() {
    return SUCCEEDED;
}

// NOT YET AUTHORED (0x1ec bytes, the largest in the unit). Called first from #create --
// likely createModel by position/size, not confirmed.
void daWmKiller_c::unk_167D30() {
    mPad_1ec[0] = 0x3;
}

// NOT YET AUTHORED (0x8c bytes). A loop constructing 10 WM_KILLERBULLET (profile 0x276)
// children into an array at this+0x214, positions from a 3-float member at this+0x1ec
// (mMotion-shaped, matching WM_ITEM's own naming) -- neither member declared yet.
void daWmKiller_c::unk_167F20() {
    mPad_1ec[0] = 0x4;
}

// NOT YET AUTHORED (0xb0 bytes). Called last from #create.
void daWmKiller_c::unk_167FB0() {
    mPad_1ec[0] = 0x5;
}

// processCutsceneCommand. Structurally confirmed: early-return on cutsceneCommandId==-1, then
// gated on !checkCutEnd() (a real dWmDemoActor_c virtual call, not guessed), then a
// isFirstFrame && cutsceneCommandId==0x38 branch calling #unk_1684A0, then an unconditional
// write to a base-class field at this+0x139 shared with WM_START's own identically-shaped
// residual (not owned by this class, raw offset write per that unit's precedent).
void daWmKiller_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) {
        return;
    }
    if (!checkCutEnd()) {
        if (isFirstFrame && cutsceneCommandId == 0x38) {
            m_208 = unk_1684A0(true);
        }
        *(bool *) ((u8 *) this + 0x139) = true;
    }
}

void daWmKiller_c::unk_1680F0() {
    mPad_1ec[0] = 0x6;
}

void daWmKiller_c::unk_1681C0() {
    mPad_1ec[0] = 0x7;
}

void daWmKiller_c::unk_168260() {
    mPad_1ec[0] = 0x8;
}

void daWmKiller_c::unk_1682B0() {
    mPad_1ec[0] = 0x9;
}

void daWmKiller_c::unk_1682D0() {
    mPad_1ec[0] = 0xa;
}

void daWmKiller_c::unk_1682F0() {
    mPad_1ec[0] = 0xb;
}

void daWmKiller_c::unk_168380() {
    mPad_1ec[0] = 0xc;
}

void daWmKiller_c::unk_168590() {
    mPad_1ec[0] = 0xd;
}

// NOT YET AUTHORED (0xe4 bytes). Called from #processCutsceneCommand with (this, true) --
// placeholder bool(bool) signature inferred only from the call site.
bool daWmKiller_c::unk_1684A0(bool b) {
    return b;
}
