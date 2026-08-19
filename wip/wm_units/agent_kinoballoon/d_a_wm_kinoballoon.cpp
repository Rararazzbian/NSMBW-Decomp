#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_wm_effect_manager.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_info.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>

/// @unofficial DRAFT. Base class settled from the target VTABLE (`lbl_2_data_456A0`), not from
/// disassembly shape: read directly out of the retail REL's own relocations (`bin/d_basesNP.rel`,
/// `dtk rel info -r`), the vtable is EXACTLY 0x78 bytes (28 slots) -- the SAME size as
/// daWmSinkShip_c's own confirmed `dWmObjActor_c`-ending vtable
/// (`bin/dtkspl/d_basesNP/obj/auto_04_00046BE0_data.txt`, `lbl_2_data_471E0`), slot-for-slot
/// identical in which entries are IMPORTED vs OVERRIDDEN. dtk's own "size:0x108" for this vtable
/// object is an OVER-MERGE (the same heuristic error class this project has hit before): bytes
/// past +0x78 are a `PpcRel24`-relocated `__ptmf_scall` thunk and unrelated trailing data, not
/// vtable slots -- `Absolute` relocations (real function pointers) stop dead at +0x74
/// (`vf78__13dWmObjActor_cFv`), matching 28 slots exactly.
///
/// Overridden slots (own function, confirmed by address falling inside 0x16a150-0x16b0f0):
/// create(+0x8), execute(+0x20), draw(+0x2c), dtor(+0x48), processCutsceneCommand(+0x60).
/// NOT overridden (resolve to dWmObjActor_c/dWmDemoActor_c's own imports): doDelete(+0x14, ->
/// fn_2_15ABC0), GetActorType(+0x58 -> GetActorType__13dWmObjActor_cFv), finalUpdate(+0x5c),
/// checkCutEnd/setCutEnd/clearCutEnd(+0x64/0x68/0x6c), vf74/vf78(+0x70/0x74).
///
/// Class hierarchy independently corroborated by the constructor (fn_2_16A180): it calls
/// `bl __ct__14dWmDemoActor_cFv` (the grand-base ctor -- `dWmObjActor_c`'s own ctor is entirely
/// inline, `dWmObjActor_c() : mResNodeIdx(-1) {}`, and folds into this ctor instead of emitting
/// its own `bl`, exactly like daWmSinkShip_c's documented case) and stores `-1` at +0x184 BEFORE
/// constructing `mAllocator` at +0x188 -- that store is `dWmObjActor_c::mResNodeIdx`'s own
/// in-class initializer being inlined, not a member this class writes itself.
///
/// sizeof == 0x208, read directly off the allocator wrapper (fn_2_16A150: `li r3, 0x208;
/// bl __nw__7fBase_cFUl`).
///
/// Member offsets read directly from the constructor (fn_2_16A180) and cross-checked against
/// create()/createModel()/calcModel():
///   dWmObjActor_c base (incl. mResNodeIdx)   ends 0x188
///   dHeapAllocator_c mAllocator               +0x188  (ctor: __ct__16dHeapAllocator_cFv)
///   nw4r::g3d::ResFile mResFile                +0x1a4  (zero-inited in ctor; stored by
///                                              createModel()'s own `getRes()` call, `stw r3,
///                                              0x1a4(r27)`, matching daWmSmallCloud_c's OWN
///                                              persistent-`mResFile` idiom rather than
///                                              daWmSinkShip_c's local-only one)
///   m3d::smdl_c mModel                         +0x1a8  (ctor: __ct__Q23m3d6smdl_cFv; ends
///                                              +0x1b4, matching sinkship's independently
///                                              confirmed sizeof(m3d::smdl_c) == 0xC)
///   m3d::anmChr_c mChrAnim[1]                  +0x1b4  (ctor: __construct_array, count 1, elem
///                                              size 0x38; ends +0x1ec)
/// 0x1ec..0x208 (0x1c B) holds several plain int/float/bool members read/written directly by
/// create()/execute()/the state handlers -- see the members below, each cited at its own site.
///
/// `mParam` (inherited, +0x4) carries TWO packed `ACTOR_PARAM_CONFIG` fields, confirmed by the
/// EXACT bit-extraction idiom used repeatedly across this unit: `extrwi r0,r5,8,8` (offset 8,
/// width 8) for what this draft calls `BalloonType`, and `clrlwi r4,r5,24` (offset 0, width 8,
/// equivalent to `& 0xff`) for `SubIndex`. Both are `ACTOR_PARAM_LOCAL`'s own compiled shape,
/// not a runtime state field -- the "state machine" look of fn_2_16A660/fn_2_16AA40 is dispatch
/// on a SPAWN-TIME CONFIG value, not on mutable state.
class daWmKinoBalloon_c : public dWmObjActor_c {
public:
    daWmKinoBalloon_c();
    virtual ~daWmKinoBalloon_c();

    virtual int create();
    virtual int execute();
    virtual int draw();

    /// @unofficial fn_2_16AA40. UNVERIFIED this round -- complex two-phase dispatch, same shape
    /// as antlion_mng's own processCutsceneCommand, with an inner dispatch on `ACTOR_PARAM
    /// (BalloonType)`. See the task report.
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    /// @unofficial fn_2_16A470. createModel().
    void createModel();
    /// @unofficial fn_2_16A5B0. calcModel() -- identical shape to sinkship/smallcloud's own.
    void calcModel();
    /// @unofficial fn_2_16A660. Dispatches on ACTOR_PARAM(BalloonType) (0-3), driving the
    /// float-triple animation-rate/frame calls and various per-type follow-up calls.
    void modeExec();
    /// @unofficial fn_2_16A7A0. Resets +0x1f0 to 0.
    void resetState();
    /// @unofficial fn_2_16A7B0. Empty -- blr.
    void procNone();
    /// @unofficial fn_2_16A7C0. "Start moving" trigger: if the current dInfo_c per-world byte at
    /// (m_instance + worldNo + 0x395) is 0x2a, calls resetState(); else resolves a course-node
    /// position via MakePointNameFromCourseNo/GetPos/GetChildNodeID and arms +0x1f0=1.
    void startMove();
    /// @unofficial fn_2_16A8A0. Interpolates position/height toward +0x1f8; on arrival plays two
    /// sounds and calls resetState().
    void moveUp();
    /// @unofficial fn_2_16A950. Reached top: sets state=2, computes a new rate, plays a sound,
    /// calls fn_2_16AF50 (end effect).
    void onReachedTop();
    /// @unofficial fn_2_16A9C0. Interpolates back down; on arrival resets +0x124 and calls
    /// resetState().
    void moveDown();
    /// @unofficial fn_2_16AD30. Writes dInfo_c's per-world 0x395/0x39f byte to 0x2a if it
    /// currently equals ACTOR_PARAM(SubIndex) -- called only when ACTOR_PARAM(BalloonType)==2.
    void markDone();
    /// @unofficial fn_2_16AD90(int subIndex). Loops fManager_c::searchBaseByProfName(WM_KINOBALLOON)
    /// until it finds a sibling instance whose ACTOR_PARAM(SubIndex) matches, or none left.
    static dBase_c *findBySubIndex(int subIndex);
    /// @unofficial fn_2_16ADF0. For each of 3 fixed sub-indices (`lbl_2_rodata_8A90`), finds the
    /// matching sibling and calls its onReachedTop().
    static void triggerAllReachedTop();
    /// @unofficial fn_2_16AE70. Same shape as triggerAllReachedTop() but calls startMove() and
    /// stops at the first match (`lbl_2_rodata_8A9C`).
    static void triggerFirstStartMove();
    /// @unofficial fn_2_16AEF0. Starts the "float" effect (fn_80103520) if not already running
    /// (+0x1fc < 0).
    void startEffect();
    /// @unofficial fn_2_16AF50. Ends the effect if running.
    void endEffect();
    /// @unofficial fn_2_16AFA0. Empty -- blr.
    void procNone2();
    /// @unofficial fn_2_16AFB0. Resets every WM_KINOBALLOON instance: two SE-related DOL calls
    /// per instance, then +0x200 = -1.
    static void resetAll();

    /// @unofficial +0x1a4. Zero-inited in the ctor; stored to by createModel()'s own `getRes()`
    /// call (persistent, matching smallcloud's own idiom rather than sinkship's local-only one).
    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::smdl_c mModel;
    m3d::anmChr_c mChrAnim[1];

    /// @unofficial +0x1e8. Set from a rodata table entry (`lbl_2_rodata_8A50+0x2c` indexed by
    /// ACTOR_PARAM(BalloonType)) in createModel(). Read nowhere else found this round.
    bool mUnk1e8;
    /// @unofficial +0x1f0. The "phase" this draft's comments call `state`: 0 = idle
    /// (resetState()), 1 = moving up (armed by startMove()), 2 = reached top (onReachedTop()).
    int mState;
    /// @unofficial +0x1f4. A computed rate (position delta / a rodata constant), consumed by
    /// moveUp()/moveDown()/onReachedTop() as an interpolation step.
    float mRate;
    /// @unofficial +0x1f8. A snapshot of mScale.x taken unconditionally at the top of every
    /// modeExec() call; used as the "target height" comparand by moveUp()/moveDown().
    float mTargetHeight;
    /// @unofficial +0x1fc. An effect handle: -1 = no effect running, else the id returned by
    /// fn_80103520. Consumed by startEffect()/endEffect().
    int mEffectHandle;
    /// @unofficial +0x200. A counter, -1 = unset. Decremented in processCutsceneCommand's
    /// BalloonType==2 branch; reset by resetAll().
    int mUnk200;
    /// @unofficial +0x204. A frame counter, set to 0x1e (30) at one call site.
    int mUnk204;

    /// @unofficial Bit offset 0, width 8 of mParam. Compiled shape: `mParam & 0xff`
    /// (`clrlwi r4,r5,24`). Used as an identifying index among sibling WM_KINOBALLOON instances.
    ACTOR_PARAM_CONFIG(SubIndex, 0, 8);
    /// @unofficial Bit offset 8, width 8 of mParam. Compiled shape: `(mParam >> 8) & 0xff`
    /// (`extrwi r0,r5,8,8`). Selects the modeExec()/processCutsceneCommand() dispatch branch.
    ACTOR_PARAM_CONFIG(BalloonType, 8, 8);

    typedef void (daWmKinoBalloon_c::*ProcFunc_t)();
    /// @unofficial `lbl_2_rodata_8A54`, decoded via the REL's own relocation stream
    /// (`bin/d_basesNP.rel`, `dtk rel info -r`): entries at +4/+0x10/+0x1c resolve to
    /// `fn_2_16A7B0`(procNone), `fn_2_16A8A0`(moveUp), `fn_2_16A9C0`(moveDown) -- indexed by
    /// mState (0/1/2) in execute().
    static const ProcFunc_t sProcTable[3];
};

ACTOR_PROFILE(WM_KINOBALLOON, daWmKinoBalloon_c, 0);

const daWmKinoBalloon_c::ProcFunc_t daWmKinoBalloon_c::sProcTable[3] = {
    &daWmKinoBalloon_c::procNone,
    &daWmKinoBalloon_c::moveUp,
    &daWmKinoBalloon_c::moveDown,
};

extern "C" int fn_80103520(dWmEffectManager_c *mgr, int effectId, m3d::bmdl_c *model,
                            const char *kind, int, int);
extern "C" void fn_80105480(dWmSeManager_c *, int);
extern "C" bool fn_80105FF0(dWmSeManager_c *, int);

daWmKinoBalloon_c::daWmKinoBalloon_c() {}
daWmKinoBalloon_c::~daWmKinoBalloon_c() {}

int daWmKinoBalloon_c::create() {
    createModel();
    mClipSphere.set(mPos, 120.0f);
    calcModel();
    mEffectHandle = -1;
    mUnk200 = -1;
    modeExec();
    mModel.setPriorityDraw(-1, 0x81);
    mUnk204 = 0;
    return SUCCEEDED;
}

int daWmKinoBalloon_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    if (csSeqMng->FUN_80915600()) {
        processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    }

    if (!csSeqMng->FUN_80915600() || csSeqMng->GetCutName() == 0x70) {
        (this->*sProcTable[mState])();
        procNone2();
    }

    if (mResNodeIdx >= 0) {
        daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    } else {
        mPos.y = -10000.0f;
    }

    mModel.play();
    calcModel();

    return SUCCEEDED;
}

int daWmKinoBalloon_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

void daWmKinoBalloon_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);
    mResFile = dResMng_c::m_instance->getRes("cobKinopio", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("cobKinopio");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1, nullptr);
    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr("cobKinopio");
    mChrAnim[0].create(resMdl, resAnmChr, &mAllocator, nullptr);
    mUnk1e8 = false;
    mChrAnim[0].setRate(0.0f);
    mChrAnim[0].setFrame(0.0f);
    mChrAnim[0].setRate(1.0f);
    mModel.setAnm(mChrAnim[0]);
    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmKinoBalloon_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmKinoBalloon_c::modeExec() {
    mTargetHeight = mScale.x;
    int type = (int)ACTOR_PARAM(BalloonType);
    switch (type) {
    case 0:
        mChrAnim[0].setRate(1.0f);
        mChrAnim[0].setFrame(0.0f);
        startEffect();
        break;
    case 1:
        mVisible = false;
        mScale = mVec3_c(0.01f, 0.01f, 0.01f);
        mChrAnim[0].setRate(0.0f);
        mChrAnim[0].setFrame(0.0f);
        break;
    case 2:
        mChrAnim[0].setRate(1.0f);
        mChrAnim[0].setFrame(0.0f);
        markDone();
        break;
    case 3:
        mVisible = false;
        mChrAnim[0].setRate(1.0f);
        mChrAnim[0].setFrame(0.0f);
        mScale = mVec3_c(0.01f, 0.01f, 0.01f);
        break;
    }
    resetState();
}

void daWmKinoBalloon_c::resetState() {
    mState = 0;
}

void daWmKinoBalloon_c::procNone() {}

void daWmKinoBalloon_c::startMove() {
    mRate = mTargetHeight / 10.0f;
    dInfo_c *info = dInfo_c::m_instance;
    daWmMap_c *map = daWmMap_c::m_instance;
    int world = dScWMap_c::m_WorldNo;
    u8 flag = *((u8 *)info + 0x395 + world);
    if ((int)flag == 0x2a) {
        resetState();
    } else {
        mVisible = true;
        mChrAnim[0].setRate(1.0f);
        char buf[8];
        int nodeIdx = dWmLib::MakePointNameFromCourseNo(world, flag, buf);
        mPos = map->GetPos(nodeIdx);
        mResNodeIdx = map->GetChildNodeID(buf, 0);
        mState = 1;
    }
}

void daWmKinoBalloon_c::moveUp() {
    if (mScale.x >= mTargetHeight) {
        startEffect();
        dWmSeManager_c::m_pInstance->playSound(0x71, mPos, 1);
        dWmSeManager_c::m_pInstance->playSound(0x6f, mPos, 1);
        resetState();
    } else {
        mScale += mVec3_c(mRate, mRate, mRate);
    }
}

void daWmKinoBalloon_c::onReachedTop() {
    mState = 2;
    mRate = -mTargetHeight / 10.0f;
    dWmSeManager_c::m_pInstance->playSound(0x70, mPos, 1);
    endEffect();
}

void daWmKinoBalloon_c::moveDown() {
    if (mScale.x <= 0.01f) {
        mVisible = false;
        resetState();
    } else {
        mScale += mVec3_c(mRate, mRate, mRate);
    }
}

void daWmKinoBalloon_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) {
        return;
    }

    if (isFirstFrame) {
        if (cutsceneCommandId == 0x71) {
            if ((int)ACTOR_PARAM(BalloonType) == 1) {
                if (mVisible && mScale.x >= mTargetHeight) {
                    setCutEnd();
                } else {
                    mVisible = true;
                    mChrAnim[0].setRate(1.0f);
                    mChrAnim[0].setFrame(0.0f);
                    mRate = mTargetHeight / 10.0f;
                    startEffect();
                    dWmSeManager_c::m_pInstance->playSound(0x71, mPos, 1);
                    dWmSeManager_c::m_pInstance->playSound(0x6f, mPos, 1);
                    mUnk204 = 0x1e;
                }
            } else if ((int)ACTOR_PARAM(BalloonType) == 2) {
                if (!mVisible) {
                    setCutEnd();
                } else {
                    dWmSeManager_c::m_pInstance->playSound(0x70, mPos, 1);
                    mRate = -mTargetHeight / 10.0f;
                    endEffect();
                }
            } else if ((int)ACTOR_PARAM(BalloonType) != 3) {
                mRate = 0.0f;
            }
        }
    }

    if (cutsceneCommandId == 0x71) {
        if ((int)ACTOR_PARAM(BalloonType) == 1) {
            if (mScale.x >= mTargetHeight) {
                setCutEnd();
            } else {
                mScale += mVec3_c(mRate, mRate, mRate);
            }
        } else if ((int)ACTOR_PARAM(BalloonType) == 2) {
            if (mScale.x <= 0.01f) {
                mVisible = false;
                setCutEnd();
            } else {
                mScale += mVec3_c(mRate, mRate, mRate);
            }
        } else {
            setCutEnd();
        }
    } else if (cutsceneCommandId == 0x9b) {
        if (fn_80105FF0(dWmSeManager_c::m_pInstance, 0x71)) {
            if (mUnk204 > 0) {
                mUnk204--;
            } else {
                setCutEnd();
            }
        }
    } else {
        mIsCutEnd = true;
    }
}

void daWmKinoBalloon_c::markDone() {
    if ((int)ACTOR_PARAM(BalloonType) != 2) {
        return;
    }
    u8 *slot = (u8 *)dInfo_c::m_instance + 0x395 + dScWMap_c::m_WorldNo;
    int idx = (int)ACTOR_PARAM(SubIndex);
    if (idx == *slot) {
        *slot = 0x2a;
        return;
    }
    slot += 0xa;
    if (idx == *slot) {
        *slot = 0x2a;
    }
}

dBase_c *daWmKinoBalloon_c::findBySubIndex(int type) {
    fBase_c *base = fManager_c::searchBaseByProfName(0x2a7, nullptr);
    while (base != nullptr) {
        if (type == (int)ACTOR_PARAM_LOCAL(((daWmKinoBalloon_c *)base)->mParam, BalloonType)) {
            break;
        }
        base = fManager_c::searchBaseByProfName(0x2a7, base);
    }
    return (dBase_c *)base;
}

void daWmKinoBalloon_c::triggerAllReachedTop() {
    const int types[3] = {1, 0, 3};
    for (u32 i = 0; i < 3; i++) {
        dBase_c *found = findBySubIndex(types[i]);
        if (found != nullptr) {
            ((daWmKinoBalloon_c *)found)->onReachedTop();
        }
    }
}

void daWmKinoBalloon_c::triggerFirstStartMove() {
    const int types[3] = {1, 0, 3};
    for (u32 i = 0; i < 3; i++) {
        dBase_c *found = findBySubIndex(types[i]);
        if (found != nullptr) {
            ((daWmKinoBalloon_c *)found)->startMove();
            break;
        }
    }
}

void daWmKinoBalloon_c::startEffect() {
    if (mEffectHandle < 0) {
        mEffectHandle = fn_80103520(dWmEffectManager_c::m_pInstance, 0x22, &mModel, "cobKinopio", 0, 0);
    }
}

void daWmKinoBalloon_c::endEffect() {
    if (mEffectHandle >= 0) {
        dWmEffectManager_c::m_pInstance->endEffect(mEffectHandle);
        mEffectHandle = -1;
    }
}

void daWmKinoBalloon_c::procNone2() {}

void daWmKinoBalloon_c::resetAll() {
    dBase_c *base = dBase_c::searchBaseByProfName(0x2a7, nullptr);
    while (base != nullptr) {
        fn_80105480(dWmSeManager_c::m_pInstance, 0x36);
        fn_80105480(dWmSeManager_c::m_pInstance, 0x37);
        ((daWmKinoBalloon_c *)base)->mUnk200 = -1;
        base = dBase_c::searchBaseByProfName(0x2a7, base);
    }
}
