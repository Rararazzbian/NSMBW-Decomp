#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_wm_effect_manager.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_unk_anim_class.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_s_chr_lib.hpp>

// @unofficial cross-module call into the DOL. Unnamed in both the REL's own
// symbol table (fn_2_16BA9C/fn_2_16BB60's callee) and the DOL's
// (bin/dtk/wiimj2d_symbols.txt: fn_80103420 = .text:0x80103420, size 0x74).
// Argument roles read off the target's actual register assignment at both
// call sites (r6 = getModelName()'s return, set via `mr r6,r3` BEFORE r3 is
// overwritten with the effect manager instance; r5 = &mModel, NOT &mPos --
// an earlier round had the 3rd/4th arguments swapped).
extern "C" void fn_80103420(dWmEffectManager_c *mgr, int effectId, m3d::mdl_c *model,
                             const char *kind, int, int);

/// @unofficial Provisional reconstruction of the shared base class for the
/// world-map Kinoko-house markers (1-Up / Red / Star). `daWmKinoko1up_c`
/// (wip/wm_kinoko_1up/complete/d_a_wm_kinoko_1up.cpp, finished, byte-exact,
/// 9/9) inherits from this and cannot land without it.
///
/// sizeof is 0x290, NOT the 0x284 an earlier survey (peer_archive/, and this
/// project's own wip/wm_units/fix/kinoko_swap.cpp) proposed. That number
/// came from crediting `mAnimResFile`/`mAnimResNames`/`mModelResName` to the
/// LEAF instead of the BASE -- but `createModel()` (fn_2_16B620, confirmed
/// non-virtually called from `create()`, which is this class's OWN vtable
/// slot 2 at fn_2_16B470) reads offsets 0x280/0x288/0x28c directly through
/// `this`, which a base-class method can only do if they are BASE members.
/// `daWmKinoko1up_c::vf84()` (in the OTHER, already-landed TU) merely FILLS
/// those inherited fields with its own resource name strings; it does not
/// own them. With that correction the base's own classInit literal
/// (`li r3, 0x290`, fn_2_16B2D0) matches the component sum exactly, with no
/// unexplained gap -- unlike the 0x284 reading, which left the classInit
/// literal 0xC bytes higher than the class it was allegedly sizing.
class daWmKinokoBase_c : public dWmObjActor_c {
public:
    enum ANIM_e {
        ANIM_0, ///< @unofficial name. Bound to the "kinoko" node.
        ANIM_1, ///< @unofficial name. Bound to the "trunk" node.
        ANIM_COUNT
    };

    daWmKinokoBase_c();
    virtual ~daWmKinokoBase_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);
    virtual void vf80();
    virtual void vf7C();
    virtual void vf84();

    /// @unofficial Defined IN-CLASS (inline/vague-linkage), not out-of-line,
    /// even though it is called by name twice below in
    /// processCutsceneCommand(). This is the fix for the unit's last defect
    /// -- see the long comment at the bottom of the file for the full
    /// mechanism this exploits and the experiments that found it.
    virtual const char *getModelName() { return "\0\0\0\0\0\0\0"; }

    void createModel();
    void calcModel();
    void mode_exec();

    dHeapAllocator_c mAllocator;          ///< @unofficial 0x188
    nw4r::g3d::ResFile mResFile;          ///< @unofficial 0x1a4. The model's own archive.
    m3d::mdl_c mModel;                    ///< @unofficial 0x1a8
    m3d::anmChr_c mChrAnim[ANIM_COUNT];   ///< @unofficial 0x1e8
    m3d::anmChrBlend_c mChrBlend;         ///< @unofficial 0x258
    nw4r::g3d::ResFile mAnimResFile;      ///< @unofficial 0x280. Always "cobKinokoRed" -- the two
                                           ///< animations are shared across every kinoko-house variant.
    int mCutsceneTimer;                   ///< @unofficial 0x284. Set to 0x3c (60) by processCutsceneCommand's
                                           ///< command-0x61 handler, then counted down to 0 on later frames.
    const char *const *mAnimResNames;     ///< @unofficial 0x288. Set by the LEAF's vf84(); consumed here.
    const char *mModelResName;            ///< @unofficial 0x28c. Set by the LEAF's vf84(); consumed here.
};
// sizeof(daWmKinokoBase_c) == 0x290, matching fn_2_16B2D0's `li r3, 0x290`.

ACTOR_PROFILE(WM_KINOKO_BASE, daWmKinokoBase_c, 0);

daWmKinokoBase_c::daWmKinokoBase_c() {}

daWmKinokoBase_c::~daWmKinokoBase_c() {}

int daWmKinokoBase_c::create() {
    createModel();
    mClipSphere.set(mPos, 120.0f);
    mode_exec();
    calcModel();
    return SUCCEEDED;
}

int daWmKinokoBase_c::execute() {
    vf7C();

    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    if (dWmLib::c_StartPointKinokoHouseID != ACTOR_PARAM(CourseNo)) {
        if (mResNodeIdx >= 0) {
            daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
        }
    }

    mChrAnim[ANIM_0].play();
    mChrAnim[ANIM_1].play();
    mModel.play();
    calcModel();

    return SUCCEEDED;
}

void daWmKinokoBase_c::vf7C() {}

int daWmKinokoBase_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmKinokoBase_c::doDelete() {
    return SUCCEEDED;
}

void daWmKinokoBase_c::createModel() {
    vf84();

    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    /// @unofficial UNREFERENCED by this function (or by any other function in
    /// this unit) yet genuinely present in the target's .data, immediately
    /// before "g3d/model.brres" -- confirmed by reading
    /// `original/d_basesNP.rel` directly at .data+0x458f0 and by every
    /// `addi rX, r27, N` in this function landing 0x18 bytes higher in the
    /// target than a draft missing it. Kept as a dead local; true origin
    /// unsettled (see agent report).
    /// TWO pointers to "cobKinokoAppear", not one. The target has a
    /// relocated word at unit offset 0x50 AND at 0x54, both resolving to
    /// lbl_2_data_458F0 ("cobKinokoAppear") -- read directly out of
    /// bin/dtkspl/.../auto_04_00044A68_data.txt. A draft with only one
    /// leaves 0x54 as four ZERO bytes, and because 0x54 is padding either
    /// way the section SIZE is identical -- check_sections.py cannot see
    /// it. This is the same size-vs-layout blind spot the vtable-offset
    /// defect exposed, in the same unit, four bytes lower.
    static const char *smc_unusedAppearName = "cobKinokoAppear";
    static const char *smc_unusedAppearName2 = "cobKinokoAppear";
    (void)smc_unusedAppearName;
    (void)smc_unusedAppearName2;

    mResFile = dResMng_c::m_instance->getRes(mModelResName, "g3d/model.brres");
    mAnimResFile = dResMng_c::m_instance->getRes("cobKinokoRed", "g3d/model.brres");

    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl(mModelResName);
    // The 4-arg INLINE wrapper, not the 5-arg overload with an explicit nullptr.
    // Routing the by-value resMdl through the wrapper anchors its stack temporary
    // in forward order; calling the 5-arg form directly leaves it in the same
    // reverse-order pool as every other pending temporary. That one difference
    // was the whole 5-slot ordering wall.
    mModel.create(resMdl, &mAllocator,
                  nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::BUFFER_RESANMVIS, 1);

    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_ONCE,
        m3d::FORWARD_ONCE
    };

    for (int i = 0; i < ANIM_COUNT; i++) {
        nw4r::g3d::ResAnmChr resAnmChr = mAnimResFile.GetResAnmChr(mAnimResNames[i]);
        mChrAnim[i].create(resMdl, resAnmChr, &mAllocator, nullptr);
        mChrAnim[i].setAnm(mModel, resAnmChr, playModes[i]);
        mChrAnim[i].setRate(0.0f);
        mChrAnim[i].setFrame(mChrAnim[ANIM_0].mFrameMax - 1.0f);
    }

    mChrBlend.create(resMdl, 8, &mAllocator, nullptr);
    mChrBlend.attach(0, &mChrAnim[ANIM_0], 1.0f);
    mChrBlend.attach(1, &mChrAnim[ANIM_1], 1.0f);
    mModel.setAnm(mChrBlend, 1.0f);

    dUnkAnimClass_c::ReleaseAnim(mChrAnim[ANIM_0]);
    dUnkAnimClass_c::ReleaseAnim(mChrAnim[ANIM_1]);

    dsChrLib::bindAnimToNode(&mModel, &mChrAnim[ANIM_0], "kinoko", nw4r::g3d::AnmObjChr::BIND_ONE);
    dsChrLib::bindAnimToNode(&mModel, &mChrAnim[ANIM_1], "trunk", nw4r::g3d::AnmObjChr::BIND_ONE);

    if (IsCourseClear() && !IsCourseFirstOmoteClear() && !IsCourseFirstUraClear()) {
        mChrAnim[ANIM_0].setFrame(0.0f);
    }

    if (dWmLib::IsAllComplete() && dWmLib::c_StartPointKinokoHouseID != ACTOR_PARAM(CourseNo)) {
        mChrAnim[ANIM_0].setFrame(mChrAnim[ANIM_0].mFrameMax - 1.0f);
    }

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmKinokoBase_c::vf84() {}

void daWmKinokoBase_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmKinokoBase_c::mode_exec() {}

/// @unofficial Best-effort reconstruction; NOT verified byte-exact. See the
/// agent report for the residual (201/209 instructions differing before this
/// rewrite; not re-measured after -- ran out of budget). Cutscene command
/// 0x3c and 0x61 are not in dCsSeqMng_c::CUTSCENE_CMD_e yet (no symbolic name
/// recovered), so they are written as raw literals -- `-ipa file` folds them
/// identically either way. The bit test `(mParam >> 16) != 0` is the raw
/// shift the target performs on the FULL `mParam`, not an `ACTOR_PARAM_LOCAL`
/// field read through the documented CourseNo/PathNode bitfields (those only
/// cover bits [0:16)) -- its meaning is unsettled.
void daWmKinokoBase_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        return;
    }

    if (isFirstFrame) {
        switch (cutsceneCommandId) {
        case 0x3c: {
            /// @unofficial No `setCutEnd()` call anywhere in this arm, on
            /// either the taken or not-taken path -- confirmed directly from
            /// the target: every exit from this arm (`bne`/`beq .L_0016BBB0`
            /// on guard failure, `b .L_0016BBB0` after the reset) jumps
            /// straight to the second dispatch. Only the 0x61 arm's FAILURE
            /// path calls it (see below) -- this was misdiagnosed as a
            /// "tail merge" in an earlier round; the two arms were never
            /// symmetric to begin with.
            if (!dWmLib::IsAllComplete() || dWmLib::c_StartPointKinokoHouseID == ACTOR_PARAM(CourseNo)) {
                if (IsCourseFirstClear()) {
                    mChrAnim[ANIM_0].mPlayMode = m3d::FORWARD_ONCE;
                    mChrAnim[ANIM_0].setRate(-1.0f);
                    mChrAnim[ANIM_0].setFrame(mChrAnim[ANIM_0].mFrameMax - 1.0f);

                    if (dWmLib::c_StartPointKinokoHouseID == ACTOR_PARAM(CourseNo)) {
                        mChrAnim[ANIM_1].mPlayMode = m3d::FORWARD_ONCE;
                        mChrAnim[ANIM_1].setRate(-1.0f);
                        mChrAnim[ANIM_1].setFrame(mChrAnim[ANIM_1].mFrameMax - 1.0f);
                    }

                    dWmEffectManager_c *effectMgr = dWmEffectManager_c::m_pInstance;
                    fn_80103420(effectMgr, 0x26, &mModel, getModelName(), 0, 0);

                    dWmSeManager_c::m_pInstance->playSound(0x26, mPos, 1);
                }
            }
            break;
        }

        case 0x61: {
            if (dWmLib::c_StartPointKinokoHouseID == ACTOR_PARAM(CourseNo) &&
                !IsCourseClear() &&
                (mParam >> 16) != 0) {
                mChrAnim[ANIM_0].setRate(1.0f);
                mChrAnim[ANIM_0].setFrame(0.0f);
                mChrAnim[ANIM_0].mPlayMode = m3d::FORWARD_ONCE;
                mChrAnim[ANIM_1].setRate(1.0f);
                mChrAnim[ANIM_1].setFrame(0.0f);
                mChrAnim[ANIM_1].mPlayMode = m3d::FORWARD_ONCE;
                mVisible = true;

                dWmEffectManager_c *effectMgr = dWmEffectManager_c::m_pInstance;
                fn_80103420(effectMgr, 0x26, &mModel, getModelName(), 0, 0);

                dWmSeManager_c::m_pInstance->playSound(0x25, mPos, 1);
                dWmSeManager_c::m_pInstance->playSound(0x1f, mPos, 1);

                mCutsceneTimer = 0x3c;
            } else {
                /// @unofficial The ONLY setCutEnd() call in this whole
                /// "first frame" section -- reached only when the 0x61
                /// guard chain fails.
                setCutEnd();
            }
            break;
        }

        default:
            break;
        }
    }
    switch (cutsceneCommandId) {
    case 3:
        setCutEnd();
        break;

    case 0x61:
        if (mChrAnim[ANIM_0].isStop()) {
            if (mCutsceneTimer > 0) {
                mCutsceneTimer--;
            } else {
                setCutEnd();
            }
        }
        break;

    case 0x3c:
        if (IsCourseFirstClear()) {
            if (mChrAnim[ANIM_0].isStop()) {
                if (dWmLib::c_StartPointKinokoHouseID == ACTOR_PARAM(CourseNo)) {
                    mVisible = false;
                    dWmLib::clearZoromeTime();
                    dInfo_c::m_instance->mStartPointKinokoHouseKind = dWmLib::getStartPointKinokoHouseKindNum();
                    dWmLib::setStartPointKinokoHouseKindNum(0);
                }
                mIsCutEnd = true;
            }
        } else {
            mIsCutEnd = true;
        }
        break;

    default:
        mIsCutEnd = true;
        break;
    }
}

/// @unofficial `getModelName()`'s body -- `return "\0\0\0\0\0\0\0";`, an
/// 8-byte all-zero string literal -- now lives IN-CLASS on the declaration
/// (see above), not out-of-line here. That is the fix for the unit's former
/// last defect: an 8-byte all-zero `.data` object that must land AFTER the
/// three weak vtables at the end of the section (unit offset 0x1b8), which
/// `getModelName()` (slot 32) returns the address of.
///
/// DO NOT move this back out-of-line, and do NOT reintroduce a NAMED static
/// (e.g. `static char smc_emptyModelName[8]`) for it, even with
/// `#pragma explicit_zero_data on`. Both were tried and both fail the same
/// way: the object lands BEFORE `__vt__16daWmKinokoBase_c` instead of after
/// it, growing `.data`'s total to the right size (0x1c0) by coincidence while
/// leaving the vtable itself 8 bytes too high (unit offset 0x90 instead of
/// 0x88) -- `check_sections.py` only compares aggregate section size and
/// cannot see this; `--layout` can.
///
/// THE MECHANISM (established this round by direct experiment, see the agent
/// report for the full probe set): MWCC emits a translation unit's `.data` in
/// (at least) three passes -- named objects, then per-function anonymous
/// literal/temp pools, then vtables -- but a function's own anonymous pool is
/// only attributed to the NORMAL (pre-vtable) pass if the function is
/// instantiated EAGERLY. A `virtual` member function that is (a) inline
/// (vague linkage / WEAK, i.e. defined in-class or in a header, not
/// out-of-line in a .cpp) and (b) reachable in this TU ONLY through its
/// vtable slot's address being taken -- never through a direct call
/// expression anywhere in the TU -- has its instantiation deferred until the
/// vtable-construction pass, and an ANONYMOUS literal used inside its body
/// (a raw string-literal expression, NOT a named variable) is pooled at that
/// point too, landing after every vtable the TU emits.
///
/// The surprising part, confirmed by direct probe, is that condition (b) does
/// NOT actually require "never called by name": what disqualified every
/// earlier attempt at this site was never the two direct calls to
/// `getModelName()` in `processCutsceneCommand()` below -- it was that the
/// object was a NAMED static (always pass 1, unconditionally, regardless of
/// which function(s) reference it -- verified separately) or that
/// `getModelName()` was still defined out-of-line (making it STRONG, and
/// strong/non-vague-linkage functions are never lazily instantiated, so
/// nothing they contain can be deferred, called-by-name or not). Once
/// `getModelName()` itself is made inline/WEAK and its own literal is
/// anonymous, ITS deferral is enough by itself; no separate never-called
/// helper function is needed to "hold" the object for it. (Named identical
/// literals were also tested for cross-function pool sharing/merging, with
/// and without `#pragma reuse_strings on` -- MWCC does not merge a
/// STRONG-bound pass-1 copy with a WEAK-bound deferred one; this is why the
/// single-inline-function shape above, not a two-function split, is what
/// works.)
///
/// Verified: `check_sections.py --layout` reports `__vt__16daWmKinokoBase_c`
/// at unit offset 0x88 (matching), `.data` at exactly 0x1c0 and `.bss` at
/// exactly 0x10 (both matching, no overshoot in either), and `verify_anon.py`
/// reports 17/17 with the new anonymous `.data` object landing at unit offset
/// 0x1b8 -- the position was checked directly, not inferred from the 17/17
/// alone (see the LIMIT note on `verify_anon.py`'s pool-symbol normalisation).

void daWmKinokoBase_c::vf80() {}
