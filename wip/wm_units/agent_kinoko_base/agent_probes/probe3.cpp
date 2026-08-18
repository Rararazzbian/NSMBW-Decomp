#pragma reuse_strings on
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
#include <game/bases/d_wm_lib_ext.hpp>
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
    virtual void vf80() {
        (void)"\0\0\0\0\0\0\0";
    }
    virtual void vf7C();
    virtual void vf84();
    virtual const char *getModelName();

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

/// @unofficial The unit's last defect, precisely localised but NOT solved this
/// round -- see the agent report for the full compiler-behaviour writeup and
/// what was ruled out.
///
/// DO NOT "fix" this by reverting to a bare `return "";`. That reads as clean
/// under all three checkers (`check_sections`/`check_vtable`/`verify_anon`),
/// but only by COINCIDENCE: the 1-byte pooled literal plus 7 bytes of
/// alignment padding before the vtable sums to the same 8 bytes the real
/// trailing object is missing, so `.data`'s TOTAL size still lands on 0x1c0
/// even though `__vt__16daWmKinokoBase_c` sits at unit offset 0x90 instead of
/// the target's 0x88. `check_sections.py` only compares aggregate section
/// size, not internal offsets, and `check_vtable.py` only compares SLOT
/// CONTENT, not the vtable object's own address -- neither catches this. The
/// `static char[8]` shape below is the worse-looking but MORE CORRECT state:
/// it gets the vtable's address right (matching the target exactly) at the
/// cost of an honest, checker-visible 8-byte `.bss` overshoot, which is the
/// one real thing left to fix.
const char *daWmKinokoBase_c::getModelName() {
    return "\0\0\0\0\0\0\0";
}
