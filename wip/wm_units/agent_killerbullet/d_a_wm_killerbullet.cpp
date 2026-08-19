#include <game/bases/d_a_wm_killerbullet.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_effect_manager.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <cmath>

ACTOR_PROFILE(WM_KILLERBULLET, daWmKillerBullet_c, 0);

// #checkParentFlag's target -- WM_KILLER's own unk_1684A0(bool), a real, already-landed-in-
// draft function at 0x1684A0 (inside daWmKiller_c's own claimed .text range). Declared via its
// exact mangled name so the linker resolves it directly once both units exist -- a landing-
// order dependency the coordinator has recorded, not something authored here.
extern "C" bool unk_1684A0__12daWmKiller_cFb(void *self, bool arg);
extern "C" bool unk_168260__12daWmKiller_cFi(void *self, int index);

// Shared game-parameter table (lbl_2_data_45428, 0x180 bytes, mixed floats/packed shorts,
// 50 referrers across the module) -- confirmed NOT this unit's own data via check_bounds.py's
// ownership check. Declared extern via the same R_<module>_<section>_<offset> convention
// already proven for a far .bss symbol (R_2_6_FE40 in WM_KILLER); section 5 is .data.
extern "C" const float R_2_5_45428[];
extern "C" const u8 R_2_5_43E34[];

// #state2's two remaining far calls, both raw DOL-absolute addresses (unowned, no mangled
// name -- modelled by call shape only, per the project's established convention for such
// calls, not guessed semantics).
extern "C" int fn_80103520(dWmEffectManager_c *effMgr, int effectId, void *model,
                            const char *nodeName, const void *angle, const void *scale);
extern "C" void fn_80103A00(dWmRotater_c *rotater, bool, int, float);

// Two standalone rodata scalars, this unit's own (both within our claimed 0x89f0-0x8a3c
// range, confirmed via check_bounds.py -- distinct from the shared external table above).
// Real values read from the retail .rodata at file offset 0x1c6600+addr.
static const float sc_60 = 60.0f;    // lbl_2_rodata_89F0
static const float sc_0 = 0.0f;      // lbl_2_rodata_89F4
static const float sc_0_001 = 0.001f; // lbl_2_rodata_8A38

// #execute's CalcShadow float constants and the state-handler table live in this unit's own
// .data/.rodata (lbl_2_data_45428, lbl_2_rodata_89F8) -- not yet named/declared here since the
// section bounds work is still open this round.

daWmKillerBullet_c::daWmKillerBullet_c() : m_1d4(false) {}

// #m_1fc's own vtable object (lbl_2_data_43E34) is genuinely a vtable -- dtk reports 0xc bytes
// but the relocations inside it run to at least +0x28 (two null words, offset-to-top and RTTI,
// followed by function pointers), and dtk's own reported object size is unreliable in both
// directions; the relocations are the authority. #dWmRotater_c is modelled as polymorphic with
// a virtual destructor, so both releases below are ordinary `delete`.
daWmKillerBullet_c::~daWmKillerBullet_c() {
    if (m_1fc != nullptr) {
        delete m_1fc;
    }
    if (mBgmSync != nullptr) {
        delete mBgmSync;
    }
}
// create(). Vtable slot 2, confirmed via check_vtable.py. Confirmed content: allocates
// #mBgmSync (a real, already-landed dWmBgmSync_c -- found by grepping include/ before
// shadow-declaring anything, per the coordinator's own precedent on agent_board), reads its
// fields from the shared table (matching agent_board's own createModel()-adjacent shape
// exactly, just sourced from #R_2_5_45428 instead of a local static array), then a second,
// still-unowned dWmRotater_c-shaped 0x40-byte allocation (#m_1fc) constructed via raw field
// writes (no landed header exists for that class), #mClipSphere setup, and two helper calls.
int daWmKillerBullet_c::create() {
    dWmBgmSync_c *bgmSync = new dWmBgmSync_c();
    mBgmSync = bgmSync;
    bgmSync->m_18 = (const s16 *) ((const u8 *) R_2_5_45428 + 0x50);
    bgmSync->m_04 = bgmSync->m_18[0] - 1;
    bgmSync->m_08 = bgmSync->m_18[1];
    m_1e4 = -1;

    unk_168C80();
    unk_168990();

    mClipSphere.set(mPos, sc_60);

    void *rotaterMem = ::operator new(0x40);
    if (rotaterMem != nullptr) {
        *(const void **) ((u8 *) rotaterMem + 0x0) = (const void *) R_2_5_43E34;
        *(float *) ((u8 *) rotaterMem + 0xc) = sc_0;
        *(int *) ((u8 *) rotaterMem + 0x20) = 0;
        *(bool *) ((u8 *) rotaterMem + 0x28) = false;
    }
    m_1fc = (dWmRotater_c *) rotaterMem;
    *(const void **) ((u8 *) rotaterMem + 0x3c) = (const u8 *) R_2_5_45428 + 0x28;

    unk_168D50();

    m_205 = false;
    m_1bc = true;
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
    mBgmSync->execute();

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

    CalcShadow(R_2_5_45428[4], R_2_5_45428[5]); // +0x10/+0x14 of the shared table
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
// #endEffectAndResetState (fn_2_168E60). Confirmed content: ends any active effect (#m_1e4)
// and resets the state index to 0.
void daWmKillerBullet_c::endEffectAndResetState() {
    if (m_1e4 >= 0) {
        dWmEffectManager_c::m_pInstance->endEffect(m_1e4);
        m_1e4 = -1;
    }
    m_1b0 = 0;
}
// #endStateOrTransition (fn_2_168F50). Confirmed content: if #checkParentFlag() is false,
// falls back to #endEffectAndResetState(); otherwise transitions to state 1 (a literal, not a
// float conversion) with a cooldown read from #unk_169510()'s own result (a genuine float->int
// truncation, `(int)`), ending any active effect and clearing speed.
void daWmKillerBullet_c::endStateOrTransition() {
    if (!checkParentFlag()) {
        m_204 = false;
        endEffectAndResetState();
    } else {
        unk_1694A0();
        void *p = unk_169510();
        float cooldown = *(const float *) ((const u8 *) p + 0x14);
        m_1b0 = 1;
        m_1b8 = (int) cooldown;
        if (m_1e4 >= 0) {
            dWmEffectManager_c::m_pInstance->endEffect(m_1e4);
            m_1e4 = -1;
        }
        clearSpeedAll();
    }
}
// #unk_1694A0. Confirmed content: snaps this bullet's own #mPos to #mParentKiller's own
// mMotion (the spawn-position member from WM_KILLER's own layout, offset 0x1ec -- confirmed
// cross-unit), shrinks #mScale to a shared near-zero constant, then calls #unk_169430().
void daWmKillerBullet_c::unk_1694A0() {
    const float *parentMotion = (const float *) ((const u8 *) mParentKiller + 0x1ec);
    mVec3_c pos(parentMotion[0], parentMotion[1], parentMotion[2]);
    mPos = pos;
    mScale.x = sc_0_001;
    mScale.y = sc_0_001;
    mScale.z = sc_0_001;
    unk_169430();
}

// #unk_169430. Confirmed content: faces this bullet toward the player along X only (Y/Z
// forced to 0), based on the X difference between the player's own position and
// #mParentKiller's.
void daWmKillerBullet_c::unk_169430() {
    mVec3_c playerPos = daWmPlayer_c::ms_instance->mPos;
    const float *parentPos = (const float *) ((const u8 *) mParentKiller + 0xac);
    mVec3_c dir(playerPos.x - parentPos[0], sc_0, sc_0);
    setDirection(dir);
}
// #unk_168C80. Confirmed content: the same createModel()-shaped sequence already established
// on WM_KILLER/WM_ITEM, using real strings read from this unit's own .data (lbl_2_data_453E8,
// file offset 0x1d0c00+addr) -- "killer"/"g3d/killer.brres" for the main model,
// "character_SV"/"g3d/model.brres" for the shadow model (CreateShadowModel's own arc/mdlName
// args reuse the SAME string, confirmed from the target's own register reuse).
void daWmKillerBullet_c::unk_168C80() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("killer", "g3d/killer.brres");
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("killer");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
    dWmActor_c::setSoftLight_Enemy(mModel);
    mAllocator.adjustFrmHeap();

    CreateShadowModel("character_SV", "g3d/model.brres", "character_SV", true);
}
void daWmKillerBullet_c::unk_168990() { m_1c0 = 22; }
// #unk_169530. Confirmed content: a tail call into WM_KILLER's own unk_168260(int) (another
// real, cross-unit-confirmed call, same landing-order dependency as #checkParentFlag) on
// #mParentKiller, with an index derived from the low byte of #mParam (0 -> 9, else value-1).
bool daWmKillerBullet_c::unk_169530() {
    u8 low = (u8) mParam;
    int index = (low == 0) ? 9 : (low - 1);
    return unk_168260__12daWmKiller_cFi(mParentKiller, index);
}
// #unk_169510. Confirmed content: indexes a per-"kind" (ACTOR_PARAM(SpawnKind)-shaped, the
// low byte of #mParam) sub-table within the shared #R_2_5_45428 table -- 0x18-byte entries,
// base offset 0x54.
void *daWmKillerBullet_c::unk_169510() {
    return (void *) ((const u8 *) R_2_5_45428 + 0x54 + (u8) mParam * 0x18);
}
void daWmKillerBullet_c::unk_1691A0() { m_1c0 = 6; }
void daWmKillerBullet_c::unk_1695E0() { m_1c0 = 7; }
void daWmKillerBullet_c::unk_1698E0() { m_1c0 = 8; }
// #unk_169F00. Confirmed content: rotates toward a shared-table angle, and on a successful
// #_procDemoJumpBase(), clears rotation, plays an explosion-shaped effect/sound pair, and
// clears #m_1d4.
bool daWmKillerBullet_c::unk_169F00() {
    rotDirectionX(*(const short *) ((const u8 *) R_2_5_45428 + 0x4c), true);
    if (_procDemoJumpBase() == false) {
        return false;
    }
    m_1d4 = false;
    mAngle.x = 0;
    mAngle3D.x = 0;
    dWmEffectManager_c::m_pInstance->playEffect(0xe, &mPos, nullptr, nullptr);
    dWmSeManager_c::m_pInstance->playSound(0x56, mPos, 1);
    return true;
}
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
// state2 (table entry 2, fn_2_169280). Confirmed content: updates mSpeedF from
// #unk_169510()'s own result, ticks #m_1e8/plays a "skl_root"-attached effect once it lapses,
// calls #unk_1695E0(), then checks distance to #mParentKiller's own position (raw offset 0xac
// -- dBaseActor_c::mPos, same offset on every actor regardless of subclass) against the shared
// table's own threshold; past it, ends or resets the state depending on #checkParentFlag().
// Finally, if #m_200's own byte at +0xd is set, resets #m_1c0 and fires fn_80103A00 on #m_1fc.
void daWmKillerBullet_c::state2() {
    m_1f9 = false;
    calcSpeed();
    posMove();

    float speedF = mSpeedF;
    void *p = unk_169510();
    mSpeedF = speedF + *(const float *) ((const u8 *) p + 0x10);

    if (m_1e8 > 0) {
        m_1e8 -= 1;
    } else if (m_1e4 < 0) {
        m_1e4 = fn_80103520(dWmEffectManager_c::m_pInstance, 0x14, &mModel, "skl_root",
                             (const u8 *) this + 0x100, (const u8 *) this + 0xdc);
    }

    unk_1695E0();

    const mVec3_c &parentPos = *(const mVec3_c *) ((const u8 *) mParentKiller + 0xac);
    if (std::fabs(mPos.distTo(parentPos)) > R_2_5_45428[0]) {
        if (!checkParentFlag()) {
            endEffectAndResetState();
        } else {
            endStateOrTransition();
        }
    }

    if (mBgmSync->m_0d) {
        m_1c0 = 0;
        fn_80103A00(m_1fc, true, -1, sc_0);
    }

    unk_1698E0();
}
// state3 (table entry 3, fn_2_1690F0). Confirmed content: a #m_1b8 cooldown counter (same
// shape as #state1's own), then once it lapses, moves and checks distance to #m_1ec (the
// stored target position) against the same shared-table threshold as #state2 -- past it, ends
// any active effect and clears a base-class flag at raw offset 0x124 (the same unnamed field
// WM_START's own draft already reaches via an identical raw cast).
void daWmKillerBullet_c::state3() {
    if (m_1b8 > 0) {
        m_1b8 -= 1;
    } else {
        calcSpeed();
        posMove();
        if (std::fabs(mPos.distTo(*(const mVec3_c *) m_1ec)) > R_2_5_45428[0]) {
            if (m_1e4 >= 0) {
                dWmEffectManager_c::m_pInstance->endEffect(m_1e4);
                m_1e4 = -1;
            }
            *(bool *) ((u8 *) this + 0x124) = false;
        }
        unk_1698E0();
    }
}
