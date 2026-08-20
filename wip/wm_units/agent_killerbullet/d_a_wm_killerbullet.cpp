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
// #unk_169550's target -- WM_KILLER's own unk_1682B0(int)/unk_1682D0(int,u8), two more
// real, already-landed-in-draft daWmKiller_c members (wip/wm_units/agent_killer/, read-only:
// its own draft.txt has both under their real mangled names). Same landing-order dependency
// convention as #checkParentFlag/#unk_169530's own externs above.
extern "C" void unk_1682B0__12daWmKiller_cFi(void *self, int index);
extern "C" void unk_1682D0__12daWmKiller_cFiUc(void *self, int index, u8 value);

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
static const float sc_half = 0.5f;   // lbl_2_rodata_8A3C -- confirmed 0.5 by reading
                                       // original/d_basesNP.rel directly (file offset 0x1cf03c)
static const float sc_1 = 1.0f;      // lbl_2_rodata_8A34 -- confirmed 1.0 by reading
                                       // original/d_basesNP.rel directly (file offset 0x1cf034)

// This unit's own uninitialised (zero-at-load) `.bss` cache, confirmed within the pinned
// bounds (0xfe10-0xfe3c, 0x2c bytes) -- a plain-`{}`-constructed `mVec3_c` has an empty ctor
// (see include/game/mLib/m_vec.hpp) so it lands in `.bss`, not `.data`, matching the target.
// Declaration ORDER matters -- the linker packs `.bss` in declaration order like `.text` --
// the confirmed order is #s_bssDir10(+0x10) then #s_bssVec1c(+0x1c) then the two param ints
// (+0x28, +0x2c) then #s_bssBox30(+0x30). Moved up here (from their original position just
// before #unk_1695E0) because #unk_168990, authored this round, is the FIRST function to read
// #s_bssVec1c/#s_bssParam28 -- a C++ file-scope static must be declared before its first use,
// and this move does not change their RELATIVE order, so `.bss` packing is unaffected.
static mVec3_c s_bssDir10; // lbl_2_bss_FE20

static mVec3_c s_bssVec1c; // lbl_2_bss_FE2C (bss+0x1c) -- the spawn-position cache #unk_168990's
                            // own case1/case2 branches copy into #mPos; also #unk_1695E0's own
                            // non-star-mode cache target.
static int s_bssParam28; // lbl_2_bss_FE38 (bss+0x28) -- (u8)#mParam, same low-byte extraction
                           // #unk_169530 already uses; indexes the per-"kind" sub-table
                           // #unk_168990's own case2 reads (same table #unk_169510 indexes,
                           // just via this cached index rather than a fresh `(u8)mParam` read).
static int s_bssParam2c; // lbl_2_bss_FE3C (bss+0x2c) -- (#mParam>>8)&0xff, the SAME
                           // second-byte extraction #unk_1693C0 already uses.
static float s_bssBox30[6]; // lbl_2_bss_FE40 (bss+0x30) -- NOT YET POPULATED by anything
                              // decoded in this unit; likely written by a cutscene/spawn setup
                              // elsewhere.

// #execute's CalcShadow float constants and the state-handler table live in this unit's own
// .data/.rodata (lbl_2_data_45428, lbl_2_rodata_89F8) -- not yet named/declared here since the
// section bounds work is still open this round.

// NOTE ON DEFINITION ORDER: the linker places .text in DEFINITION order, and every name below
// carries its real target address -- so every function body from here down MUST appear in
// ascending target-address order. Verified with wip/wm_units/check_fn_order.py. Unauthored
// functions are simply absent (not stubbed in the wrong slot); when one is authored, it goes
// into its correct address slot, not at the end of the file.

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

// #unk_168990 (fn_2_168990). REPLACES a fake stub this round -- real content. Always resets
// via #endEffectAndResetState() first, then a 3-way switch on `(int)(mParam>>16)` (the SAME
// upper-half extraction #draw/#execute/#processCutsceneCommand already use):
// - case 0: finds #mParentKiller via #unk_1693C0()'s own search loop, then calls #unk_1694A0()
//   (which snaps #mPos to #mParentKiller's own spawn position and faces along X).
// - case 1: faces via #setDirection(#s_bssDir10), snaps #mPos to #s_bssVec1c (this unit's own
//   `.bss` spawn-position cache, the `+0x1c` slot), and sets #mScale to a shared-table scalar
//   (index 1, same value on all three axes -- the SAME constant #unk_1691A0 already uses).
// - case 2: the SAME facing/#mPos/#mScale sequence as case 1, PLUS #mSpeedF from the same
//   per-"kind" sub-table #unk_169510 indexes (base+0x54, 0x18-byte stride, `+0xc` field) but
//   keyed by #s_bssParam28 (a cached index, not a fresh `(u8)mParam` read), #m_1ec snapshotted
//   from #s_bssVec1c, and finally #unk_169080().
void daWmKillerBullet_c::unk_168990() {
    endEffectAndResetState();
    int kind = (int) (mParam >> 16);
    if (kind == 0) {
        mParentKiller = unk_1693C0();
        unk_1694A0();
    } else if (kind == 1) {
        setDirection(s_bssDir10);
        mPos = s_bssVec1c;
        mScale.x = R_2_5_45428[1];
        mScale.y = R_2_5_45428[1];
        mScale.z = R_2_5_45428[1];
    } else if (kind == 2) {
        setDirection(s_bssDir10);
        mPos = s_bssVec1c;
        mScale.x = R_2_5_45428[1];
        mScale.y = R_2_5_45428[1];
        mScale.z = R_2_5_45428[1];
        mSpeedF = *(const float *) ((const u8 *) R_2_5_45428 + 0x54 + s_bssParam28 * 0x18 + 0xc);
        m_1ec[0] = mPos.x;
        m_1ec[1] = mPos.y;
        m_1ec[2] = mPos.z;
        unk_169080();
    }
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

// #unk_168D50 (fn_2_168D50). REPLACES a fake stub this round -- the SAME PSMTXTrans/ZXYrotM/
// setLocalMtx/setScale/calc(false) idiom already landed on multiple real siblings (confirmed by
// reading `source/d_basesNP/bases/d_a_wm_antlion.cpp`'s own `calc()`, which is BYTE-IDENTICAL in
// shape: `mMatrix.trans(pos); mMatrix.ZXYrotM(angle); mModel.setLocalMtx(&mMatrix);
// mModel.setScale(...); mModel.calc(false);`), except #mScale gets an extra offset added first
// -- #m_1fc's own `m_0c` field (a `float`, newly modelled -- see the shadow header's own note)
// times #sc_1 (confirmed 1.0, read directly from the retail `.rel`), added to all three axes.
// Called unconditionally from both #create's own tail and #execute's own tail.
void daWmKillerBullet_c::unk_168D50() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mVec3_c scale = mScale;
    float offset = m_1fc->m_0c;
    mVec3_c newScale = scale + offset * mVec3_c(sc_1, sc_1, sc_1);
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(newScale);
    mModel.calc(false);
}

// #endEffectAndResetState (fn_2_168E60). Confirmed content: ends any active effect (#m_1e4)
// and resets the state index to 0.
void daWmKillerBullet_c::endEffectAndResetState() {
    if (m_1e4 >= 0) {
        dWmEffectManager_c::m_pInstance->endEffect(m_1e4);
        m_1e4 = -1;
    }
    m_1b0 = 0;
}

// state0 (table entry 0, fn_2_168EB0). Confirmed content: only acts when the low ACTOR_PARAM
// half of mParam is zero-shifted (i.e. (mParam>>16)==0, checked via srwi.) AND #checkParentFlag()
// is true.
void daWmKillerBullet_c::state0() {
    if ((int) (mParam >> 16) == 0 && checkParentFlag()) {
        endStateOrTransition();
    }
}

// #unk_168F00 (fn_2_168F00). Confirmed content: a true tail call (leaf -- no frame at all in
// the target, matching a `b`, not `bl`+`blr`): sets #m_1b0 to 4, then falls straight into
// #unk_169E10 as its own last action, so #unk_169E10's own return value (none; void) becomes
// this function's.
void daWmKillerBullet_c::unk_168F00() {
    m_1b0 = 4;
    unk_169E10();
}

// state4 (table entry 4, fn_2_168F10). Confirmed content: calls fn_2_169F00 (not yet named --
// NOT the same as #checkParentFlag) and, if it returns true, ends the state.
void daWmKillerBullet_c::state4() {
    if (unk_169F00()) {
        endStateOrTransition();
    }
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

// #unk_169080 (fn_2_169080). PARKED at 13/28 differing (target 28 lines, SAME size). Content is
// confirmed and complete -- transitions to state 3 (#m_1b0=3), resets #m_1c0, reloads #m_1b8
// from the shared table (+0x4e, a packed short), and fires the same "skl_root"-attached effect
// #state2's own body already fires (identical #fn_80103520 call shape, effect id 0x14,
// angle/scale from the same #mAngle/#mScale raw-offset pair) -- every residual line is a
// register-allocation/scheduling choice (which GPR holds which small constant, and whether the
// 0x4e short-load is hoisted next to its base-address computation or held back next to its own
// store), not a content or structure difference. Three genuinely different statement orderings
// tried (m_1c0/m_1b8/m_1b0; m_1c0/m_1b0/m_1b8; m_1b8/m_1c0/m_1b0, kept below as the best) all
// land in the 13-14 range -- the scheduler does not appear to be steerable by source order alone
// here. Scouted as one of #unk_168990's own callees; not yet wired into that still-unauthored
// caller.
void daWmKillerBullet_c::unk_169080() {
    m_1b8 = *(const short *) ((const u8 *) R_2_5_45428 + 0x4e);
    m_1c0 = 0;
    m_1b0 = 3;
    m_1e4 = fn_80103520(dWmEffectManager_c::m_pInstance, 0x14, &mModel, "skl_root",
                         (const u8 *) this + 0x100, (const u8 *) this + 0xdc);
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

// #unk_1691A0 (fn_2_1691A0). REPLACES a fake stub this round -- real content. Reads
// #unk_169510's own per-"kind" sub-table entry (+0xc, a float) into #mSpeedF, sets #mScale to a
// shared-table scalar (index 1, same value on all three axes), transitions to state 2 (also
// resetting #m_1bc/#m_1c0/#m_1cc/#m_1f8), writes this bullet's own low mParam byte into
// #mParentKiller's own `m_20c` field (a cross-unit write -- #unk_169550 later reads that SAME
// field back), faces the bullet at #mParentKiller's own (0x1f8,0x1fc,0x200) fields (the SAME
// delta-from-those-three-offsets idiom already used in #unk_1695E0), and finally calls
// WM_KILLER's own `unk_1682F0()` (another real, cross-unit-confirmed call -- HANDOFF already
// records it as symbol-naming-only against ITS OWN unit) before setting #m_1e8/#m_1f9.
// Result: 14/55 differing, SAME size. All but 2 lines are naming-only (calls to #unk_169510 and
// WM_KILLER's own `unk_1682F0` by real name); the rest is a minor store-scheduling shuffle (the
// #mSpeedF store is deferred a few instructions relative to the target, which stores it
// immediately) -- not re-attempted further this round, budget went to the other stubs.
extern "C" void unk_1682F0__12daWmKiller_cFv(void *self);
void daWmKillerBullet_c::unk_1691A0() {
    void *p = unk_169510();
    mSpeedF = *(const float *) ((const u8 *) p + 0xc);
    mScale.x = R_2_5_45428[1];
    mScale.y = R_2_5_45428[1];
    mScale.z = R_2_5_45428[1];
    m_1b0 = 2;
    m_1bc = false;
    m_1c0 = 0;
    m_1cc = 0;
    *(int *) ((u8 *) mParentKiller + 0x20c) = (u8) mParam;
    m_1f8 = false;

    const u8 *pk = (const u8 *) mParentKiller;
    mVec3_c raw(*(const float *) (pk + 0x1f8), *(const float *) (pk + 0x1fc),
                *(const float *) (pk + 0x200));
    mVec3_c delta = raw - mPos;
    setDirection(delta);

    unk_1682F0__12daWmKiller_cFv(mParentKiller);
    m_1e8 = 10;
    m_1f9 = true;
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

// #unk_1693C0 (fn_2_1693C0). Confirmed content: a `dBase_c::searchBaseByProfName(WM_KILLER,
// ...)` loop -- searching forward from the previous hit until one is found whose OWN low
// mParam byte matches this bullet's own second-from-bottom mParam byte, or the search runs
// out (returns nullptr either way, from the search itself). Scouted as #unk_168990's own
// case-0 `mParentKiller` finder; not yet wired into that still-unauthored caller.
dBase_c *daWmKillerBullet_c::unk_1693C0() {
    // PARKED at 14/25 differing (target 27 lines). Three genuinely different attempts: (1) a
    // `(u8)==(u8)` compare emitted `cmplw` against the target's own `cmpw` -- signedness lever;
    // (2, kept below) masking through `int` instead fixed the compare instruction but a
    // structural residual remains -- the target's own loop-exit path re-materialises `li r3,0`
    // AND falls through to the SAME epilogue the "found" path uses, while this compiles to two
    // separate return sites; (3) folding the break condition into the `while` test itself
    // regressed to 19 differing. Content (the search target, the byte fields compared, the
    // profile) is confirmed; the residual is pure control-flow shape.
    dBase_c *found = dBase_c::searchBaseByProfName(fProfile::WM_KILLER, nullptr);
    while (found != nullptr) {
        if ((int) (found->mParam & 0xff) == (int) ((mParam >> 8) & 0xff)) {
            break;
        }
        found = dBase_c::searchBaseByProfName(fProfile::WM_KILLER, found);
    }
    return found;
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

// #checkParentFlag(). Confirmed content: a tail call to WM_KILLER's own unk_1684A0(false) on
// #mParentKiller -- a cross-unit-confirmed call, not guessed (see the extern declaration above
// and its own note on the landing-order dependency this creates).
bool daWmKillerBullet_c::checkParentFlag() {
    return unk_1684A0__12daWmKiller_cFb(mParentKiller, false);
}

// #unk_169510. Confirmed content: indexes a per-"kind" (ACTOR_PARAM(SpawnKind)-shaped, the
// low byte of #mParam) sub-table within the shared #R_2_5_45428 table -- 0x18-byte entries,
// base offset 0x54.
void *daWmKillerBullet_c::unk_169510() {
    return (void *) ((const u8 *) R_2_5_45428 + 0x54 + (u8) mParam * 0x18);
}

// #unk_169530. Confirmed content: a tail call into WM_KILLER's own unk_168260(int) (another
// real, cross-unit-confirmed call, same landing-order dependency as #checkParentFlag) on
// #mParentKiller, with an index derived from the low byte of #mParam (0 -> 9, else value-1).
bool daWmKillerBullet_c::unk_169530() {
    u8 low = (u8) mParam;
    int index = (low == 0) ? 9 : (low - 1);
    return unk_168260__12daWmKiller_cFi(mParentKiller, index);
}

// #unk_169550 (fn_2_169550). Confirmed content: this is the CROSS-UNIT function WM_KILLER's own
// execute() already calls as a free function on a raw `dWmActor_c*` (recorded in HANDOFF as a
// landing-order dependency, `R_2_1_169550`) -- it is actually an ordinary member here, reached
// entirely through #mParentKiller. Clears (#unk_1682D0 with value 0) and re-flags
// (#unk_1682B0) EVERY one of WM_KILLER's own 10 pre-allocated bullet children by index, then
// activates the NEXT one in round-robin order (#mParentKiller's own `m_20c`, confirmed `int` in
// agent_killer's own shadow header, wraps 9->0) via one more #unk_1682D0 call with value 1.
void daWmKillerBullet_c::unk_169550() {
    for (int i = 0; i < 10; i++) {
        unk_1682D0__12daWmKiller_cFiUc(mParentKiller, i, 0);
        unk_1682B0__12daWmKiller_cFi(mParentKiller, i);
    }
    int next = *(const int *) ((const u8 *) mParentKiller + 0x20c);
    if (next == 9) {
        unk_1682D0__12daWmKiller_cFiUc(mParentKiller, 0, true);
    } else {
        unk_1682D0__12daWmKiller_cFiUc(mParentKiller, next + 1, true);
    }
}

// #unk_1695E0 (fn_2_1695E0). REPLACES a fake stub this round -- real content, size-exact
// (116/116 lines), 67 differing after two structural fixes this round (bss declaration order so
// #s_bssBox30 lands at +0x30 not +0x20; the star/non-star `if`/`else` polarity, which the
// target places non-star FIRST inline and star-mode as a trailing goto-style block). Remaining
// residual: (a) 5 lines of pure symbol naming (#unk_1697B0/#unk_169DA0/#unk_168F00 called by
// real name); (b) the this+player midpoint calc hits the SAME float-argument-evaluation-order
// class already parked on #unk_169E10/#unk_1697B0 (not re-attempted -- matches a known wall);
// (c) minor float-register renumbering in the #mParentKiller delta computation (content AND
// subtraction order -- Z,Y,X, matching a right-to-left `operator-` evaluation -- both confirmed
// correct; only the specific register NUMBERS differ, not what they hold).
//
// Content: an early-out on #m_1d4
// (already rotating -- return immediately), then a two-part "is the player attacking this
// bullet" test (#unk_1697B0 against a `.bss` box at \p s_bssBox30, THEN #unk_169DA0's own
// distance check), gated further on `daWmPlayer_c::isWalkToAttackPoint()` (a real static, added
// to the shadow player header alongside `attackMapEnemy`, neither landed yet). On a match, sets
// #m_1d4 and either (non-star) plays an explosion-shaped effect at the this/player midpoint,
// caches #mPos/#mParam pieces into the `.bss` block for later consumption, computes a delta
// vector from #mParentKiller's own (0x1f8,0x1fc,0x200) fields minus its own #mPos into
// #s_bssDir10, and triggers a cutscene via
// `dCsSeqMng_c::ms_instance->FUN_801017c0(SMC_DEMO_KILLER, nullptr, nullptr, 0x80)`; or
// (star mode) just `ms_instance->attackMapEnemy(true)` then #unk_168F00.
// (#s_bssVec1c/#s_bssParam28/#s_bssParam2c/#s_bssBox30 are declared up near #s_bssDir10, at the
// top of the file -- moved there so #unk_168990, which is defined earlier in `.text` than this
// function, can also see them; see that declaration's own note.)
void daWmKillerBullet_c::unk_1695E0() {
    if (m_1d4) {
        return;
    }
    bool matched = false;
    if (unk_1697B0(s_bssBox30) && unk_169DA0()) {
        matched = true;
    }
    if (!matched) {
        return;
    }
    if (!daWmPlayer_c::isWalkToAttackPoint()) {
        return;
    }
    m_1d4 = matched;
    if (!daWmPlayer_c::isPlayerStarMode()) {
        mVec3_c playerPos = daWmPlayer_c::ms_instance->mPos;
        mVec3_c midPos = (mPos + playerPos) * sc_half;
        dWmEffectManager_c::m_pInstance->playEffect(0xf, &midPos, nullptr, nullptr);

        s_bssVec1c = mPos;
        s_bssParam28 = (u8) mParam;
        s_bssParam2c = (int) ((mParam >> 8) & 0xff);

        const u8 *pk = (const u8 *) mParentKiller;
        mVec3_c raw(*(const float *) (pk + 0x1f8), *(const float *) (pk + 0x1fc),
                    *(const float *) (pk + 0x200));
        const mVec3_c &parentPos = *(const mVec3_c *) (pk + 0xac);
        mVec3_c delta = raw - parentPos;
        s_bssDir10 = delta;

        dCsSeqMng_c::ms_instance->FUN_801017c0(dCsSeqMng_c::SMC_DEMO_KILLER, nullptr, nullptr, 0x80);
    } else {
        daWmPlayer_c::ms_instance->attackMapEnemy(true);
        unk_168F00();
    }
}

// #unk_1697B0 (fn_2_1697B0). Confirmed content: an early-out "did this bullet actually move"
// check (comparing #mPos against #mLastPos on both X and Z, via the SAME fcmpu-then-fcmpo idiom
// used everywhere else moved-vs-not is tested in this project -- the specific quadrant value
// (2/3/4/5) is computed but genuinely unused beyond the ==6 "didn't move" case, matching the
// target's own dead code exactly rather than a simplified equivalent), then an expanded-AABB
// overlap test: \p box's own two corner points (six floats, `{x,y,z}` pairs) average to a
// midpoint (#sc_half, confirmed 0.5 by reading the retail .rel directly), and \p this is
// "inside" if its own X and Z distance from that midpoint are each under half the shared
// table's own X/Z sizes (#R_2_5_45428 indices 2/3, both 60.0) plus the box's own half-extents.
// PARKED at 73/76 differing (target 76 lines, draft 78). Content and the overall two-part
// shape (moved-check, then expanded-AABB test) are confirmed; two distinct residual axes, both
// resistant to three genuinely different rewrites: (1) the Z-branch's `==`-then-`<=` compare on
// the SAME operand pair fuses into a `cror`-combined single test in every phrasing tried
// (nested if/else with an explicit `quadrant=6` leaf; a `!=`-guarded ternary; nested if/else
// with the default hoisted to the declaration) -- yet the target's OWN `==`-then-`<=` pair on
// the X operands, one`if`-level up, never fuses, so the trigger is nesting depth or block
// emptiness, not the comparison shape itself, and no source rewrite reached it; (2) the AABB
// half's `mVec3_c sum(...)`/`mid` locals hit the SAME float-argument-evaluation-order residual
// already parked on #unk_169E10 (register/load scheduling, not content). Not re-attempted this
// round per the "no fourth variant on a measured wall" rule.
bool daWmKillerBullet_c::unk_1697B0(const float *box) {
    int quadrant = 6;
    if (mPos.x == mLastPos.x) {
        if (mPos.z == mLastPos.z) {
            // quadrant stays 6
        } else if (mPos.z <= mLastPos.z) {
            quadrant = 3;
        } else {
            quadrant = 2;
        }
    } else if (mPos.x <= mLastPos.x) {
        quadrant = 4;
    } else {
        quadrant = 5;
    }
    if (quadrant == 6) {
        return false;
    }

    mVec3_c sum(box[0] + box[3], box[1] + box[4], box[2] + box[5]);
    mVec3_c mid = sum * sc_half;

    float halfWidthX = mid.x - box[0];
    float dx = mPos.x - mid.x;
    if (std::fabs(dx) < R_2_5_45428[2] * sc_half + std::fabs(halfWidthX)) {
        float halfWidthZ = mid.z - box[2];
        float dz = mPos.z - mid.z;
        if (std::fabs(dz) < R_2_5_45428[3] * sc_half + std::fabs(halfWidthZ)) {
            return true;
        }
    }
    return false;
}

// #unk_1698E0 (fn_2_1698E0). REPLACES a fake stub this round -- a 5-state wobble state machine
// (#m_1c0 = 0..4) run once per frame from both #state2's and #state3's own tails, on top of an
// unconditional (result-discarded) #calcRotate call through #m_1fc. State 0 initialises #m_1c4
// from the shared table and zeroes the two counters. State 1 ramps #m_1c4 up (capped) and feeds
// it to #unk_169B80 every frame, tallying wraps into #m_1cc until BOTH the tally and #mAngle's
// own z exceed shared thresholds, at which point it hands off to state 2. States 2/3 decay
// #m_1c4 toward zero from either side and, once it reaches zero, set #m_1d0 (the direction for
// state 4) and hand off to state 4. State 4 dispatches on #m_1d0 into two near-mirror-image
// branches that push #m_1c4 further in the same direction (clamped) until #mAngle's own z
// crosses a threshold, at which point it hands back to state 2 or 3 -- a real oscillation.
// Every `shared[N]` below is a `short` read from #R_2_5_45428, distinct from the many `float`
// reads elsewhere in this unit -- the table is genuinely mixed-width, already established.
//
// PARKED at 163/167 differing (target 167 lines, draft 170) -- but content, field types, control
// flow, and every constant are all CONFIRMED correct: states 2 and 3, which never touch the
// shared table, already diff line-for-line against the target modulo a constant address offset.
// The residual is ONE register-allocation choice: state 1 (and state 4) read #R_2_5_45428 5+
// times each, and the compiler caches the table's base address in r30 (a callee-saved register,
// hence the extra `stw r30,0x8(r1)`/restore in the prologue/epilogue) for the whole function,
// where the target re-loads `lis`/`addi` fresh at every single read and needs no r30 at all --
// this shifts every address after case 0 by a near-constant few instructions, cascading through
// the whole function's branch targets. Two genuinely different attempts, IDENTICAL result both
// times: (1) the #R5S macro used as-is; (2) each read group wrapped in its own nested `{ }`
// block to try to shorten the perceived live range. This is the SAME "too many saved registers"
// class already on record for this unit's own #execute (HANDOFF: "TOO MANY saved registers means
// the INVERSE lever") -- a genuine wall, not a source-shape problem reachable from here. A third
// attempt was not made this round in favour of leaving budget for the other 3 stubs.
#define R5S(off) (*(const short *) ((const u8 *) R_2_5_45428 + (off)))
void daWmKillerBullet_c::unk_1698E0() {
    m_1fc->calcRotate();
    switch (m_1c0) {
    case 0:
        m_1c4 = R5S(0x1a);
        m_1c0 = 1;
        m_1c8 = 0;
        m_1cc = 0;
        break;
    case 1: {
        short cap = R5S(0x20);
        int v = m_1c4 + R5S(0x1c);
        if (v < cap) {
            cap = v;
        }
        m_1c4 = cap;
        if (unk_169B80(m_1c4)) {
            m_1cc += 1;
        }
        if (m_1cc > R5S(0x24)) {
            if (mAngle.z > R5S(0x22)) {
                m_1c0 = 2;
                m_1c6 = R5S(0x1c);
            }
        }
        break;
    }
    case 2: {
        short doubled = m_1c6 * 2;
        if (m_1c4 > doubled) {
            m_1c4 = m_1c4 - doubled;
        } else {
            m_1c4 = 0;
        }
        unk_169B80(m_1c4);
        if (m_1c4 == 0) {
            m_1d0 = 1;
            m_1c0 = 4;
        }
        break;
    }
    case 3: {
        short doubled = m_1c6 * 2;
        if (m_1c4 < doubled) {
            m_1c4 = m_1c4 + doubled;
        } else {
            m_1c4 = 0;
        }
        unk_169B80(m_1c4);
        if (m_1c4 == 0) {
            m_1d0 = 0;
            m_1c0 = 4;
        }
        break;
    }
    case 4:
        if (m_1d0 == 1) {
            short cap = (short) -R5S(0x20);
            int v = m_1c4 - R5S(0x1e);
            if (v < cap) {
                v = cap;
            }
            m_1c4 = v;
            unk_169B80(m_1c4);
            if (mAngle.z < (short) -R5S(0x22)) {
                m_1c0 = 3;
                m_1c6 = R5S(0x1e);
            }
        } else {
            short cap = R5S(0x20);
            int v = m_1c4 + R5S(0x1e);
            if (v > cap) {
                v = cap;
            }
            m_1c4 = v;
            unk_169B80(m_1c4);
            if (mAngle.z > R5S(0x22)) {
                m_1c0 = 2;
                m_1c6 = R5S(0x1e);
            }
        }
        break;
    }
}
#undef R5S

// #unk_169B80 (fn_2_169B80). Confirmed content: `#m_1c8 += delta`, wraps at 0x10000 (adds
// 1 after subtracting 0x10000, matching the target's own `subis`+`addi 1` shape exactly, not
// a plain modulo), returns whether it wrapped, and mirrors the low 16 bits of the (possibly
// wrapped) result into #mAngle's own `z` component. #m_1c8 itself was plain padding until this
// function's own `stw`/`lwz` proved it a real `int` -- see the shadow header's own note.
bool daWmKillerBullet_c::unk_169B80(int delta) {
    m_1c8 += delta;
    bool wrapped = false;
    if (m_1c8 > 0xffff) {
        m_1c8 = m_1c8 - 0x10000 + 1;
        wrapped = true;
    }
    mAngle.z = (short) m_1c8;
    return wrapped;
}

// #processCutsceneCommand (fn_2_169BC0). Vtable slot 24, confirmed via check_vtable.py. PARKED
// at 49/117 differing (target 117 lines, draft 111) after two structural fixes this round: (1)
// both 3-way dispatches on \p cutsceneCmd are genuine `switch` statements, NOT if/else-if chains
// -- an if/else-if chain interleaves each compare+branch with its own body, but the target lays
// out ALL THREE compares up front then the case bodies after, the documented switch-vs-if-chain
// tell; rewriting from if/else-if to `switch` dropped 102->49 differing outright. (2) `#calcRotate`
// is a genuine HEADER DEFECT FIX: was `void`, but this call site consumes its result
// (`if (!m_1fc->calcRotate()) return;`) -- changed to `bool` in the shadow header; #execute's own
// already-MATCHED call, which discards the result, is unaffected (a discarded value's type never
// changes codegen). Remaining residual, tried both a named-intermediate and a fully-inlined
// phrasing (identical result both times): the four `this+0x60`-then-`+0x64`/`+0x68` raw
// function-pointer-table walks (modelled as raw casts, NOT an invented type, matching the
// destructor's own precedent for #m_1fc) consistently allocate r4-then-r12 here where the target
// reuses r12 for both loads -- a register-allocation choice that resists both phrasings tried,
// plus a few `sc_0`-vs-"SYM0" naming-only lines from float-constant pooling. Gate call through a
// raw function-pointer table at `this+0x60` (offset 0x64), inherited from the base class. On
// `isFirstFrame`, a 3-way switch on \p cutsceneCmd (86/89/0x9a, the first two matching real
// `dCsSeqMng_c::CUTSCENE_CMD_e` entries -- 0x9a has none). A SECOND, independent switch on the
// SAME \p cutsceneCmd then runs regardless of `isFirstFrame`.
void daWmKillerBullet_c::processCutsceneCommand(int cutsceneCmd, bool isFirstFrame) {
    if (cutsceneCmd == dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        return;
    }
    typedef int (*GateFn_t)(daWmKillerBullet_c *);
    if ((*(GateFn_t *) ((u8 *) *(void **) ((u8 *) this + 0x60) + 0x64))(this)) {
        return;
    }

    typedef void (*Vtbl68Fn_t)(daWmKillerBullet_c *);

    if (isFirstFrame) {
        switch (cutsceneCmd) {
        case dCsSeqMng_c::CUTSCENE_CMD_86:
            if (m_1d4) {
                fn_80103A00(m_1fc, true, -1, sc_0);
                dWmSeManager_c::m_pInstance->playSound(0x64, mPos, 1);
            }
            break;
        case dCsSeqMng_c::CUTSCENE_CMD_89:
            if ((int) (mParam >> 16) == 1) {
                unk_169E10();
            } else {
                (*(Vtbl68Fn_t *) ((u8 *) *(void **) ((u8 *) this + 0x60) + 0x68))(this);
            }
            break;
        case 0x9a:
            m_205 = dCsSeqMng_c::ms_instance->GetCutArg0() != 0;
            break;
        }
    }

    switch (cutsceneCmd) {
    case dCsSeqMng_c::CUTSCENE_CMD_86:
        if (m_1d4) {
            if (!m_1fc->calcRotate()) {
                return;
            }
            fn_80103A00(m_1fc, false, -1, sc_0);
        }
        (*(Vtbl68Fn_t *) ((u8 *) *(void **) ((u8 *) this + 0x60) + 0x68))(this);
        break;
    case dCsSeqMng_c::CUTSCENE_CMD_89:
        if ((int) (mParam >> 16) == 1 && unk_169F00()) {
            *(bool *) ((u8 *) this + 0x124) = false;
            (*(Vtbl68Fn_t *) ((u8 *) *(void **) ((u8 *) this + 0x60) + 0x68))(this);
        }
        break;
    default:
        *(bool *) ((u8 *) this + 0x139) = true;
        break;
    }
}

// #unk_169DA0 (fn_2_169DA0). Confirmed content: null-checks #mParentKiller, then compares
// `mPos.distTo(mParentKiller->mPos)` (the SAME `distTo` idiom #state2/#state3 already use)
// against a shared threshold constant. The constant lives OUTSIDE this unit's own rodata
// bounds (0x89f0-0x8a3c) -- `lbl_2_rodata_89B8` is a second, distinct shared table from the
// one #sc_60/#sc_0/#sc_0_001 already claim, declared `extern` via the `.rodata`-is-section-4
// convention (`R_<module>_4_<offset>`), matching #R_2_5_45428's own precedent for `.data`.
extern "C" const float R_2_4_89B8[];
bool daWmKillerBullet_c::unk_169DA0() {
    // PARKED at 13/26 differing (target 26 lines, SAME size). Three genuinely different
    // attempts: (1) loading the threshold constant AFTER the two calls dropped the f31
    // save/restore entirely (frame -0x10 vs the target's own -0x20) -- confirmed real, not
    // naming; (2, kept below) naming the threshold as its own local before the calls restored
    // the exact frame/save shape and got the size exact, residual now pure branch polarity plus
    // return-path merging; (3) a single-exit `bool result` style regressed to 17 differing (the
    // null-check's `beq`/`bne` polarity and the two return sites resist both an early-return AND
    // a single-exit rewrite). Content (the null check, #distTo, the shared threshold) confirmed.
    if (mParentKiller == nullptr) {
        return false;
    }
    float threshold = R_2_4_89B8[5]; // +0x14
    const mVec3_c &parentPos = *(const mVec3_c *) ((const u8 *) mParentKiller + 0xac);
    return mPos.distTo(parentPos) < threshold;
}

// #unk_169E10 (fn_2_169E10). Confirmed content: builds an offset spawn position
// (mPos.x+shared[0xc], mPos.y+shared[0xd], mPos.z unchanged), computes a frame count and two
// scales from #mScale.x times two more shared-table entries, calls the real base-class
// `_initDemoJumpBase` (mangled `RC7mVec3_ciifffRC7mVec3_c` -- confirmed against the landed
// header's own overload), then saves/restores #mAngle3D around a `setDirection` call whose dir
// arg is #s_bssDir10 (this unit's own `.bss` cache, NOT `mVec3_c::Ey` -- that's
// `_initDemoJumpBase`'s own dir arg instead), and finally clears any active effect (#m_1e4).
void daWmKillerBullet_c::unk_169E10() {
    // PARKED at 19/58 differing (target 58 lines, SAME size). Content is fully confirmed --
    // every residual is which shared-table float gets loaded in which order relative to
    // #mPos's own loads (a pure scheduling/evaluation-order artifact of the 3-arg constructor);
    // a field-by-field `y;z;x` rewrite (matching the target's own STORE order) tried as a second
    // attempt regressed to 21. #s_bssDir10's own two lines are naming-only (MATCH-equivalent).
    mVec3_c pos(mPos.x + R_2_5_45428[0xc], mPos.y + R_2_5_45428[0xd], mPos.z);
    short frames = *(const short *) ((const u8 *) R_2_5_45428 + 0x40);
    float startScale = mScale.x * R_2_5_45428[0x11];
    float targetScale = mScale.x * R_2_5_45428[0x12];
    _initDemoJumpBase(pos, 0, frames, R_2_5_45428[0xf], startScale, targetScale, mVec3_c::Ey);

    mAng3_c savedAngle3D = mAngle3D;
    setDirection(s_bssDir10);
    mAngle3D = savedAngle3D;

    if (m_1e4 >= 0) {
        dWmEffectManager_c::m_pInstance->endEffect(m_1e4);
        m_1e4 = -1;
    }
}

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
