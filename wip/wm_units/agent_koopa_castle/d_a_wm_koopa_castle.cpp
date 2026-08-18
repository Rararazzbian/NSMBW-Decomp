#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_koopa_castle.hpp>
#include <game/bases/d_wm_lib_koopa_castle_ext.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/framework/f_manager.hpp>

ACTOR_PROFILE(WM_KOOPA_CASTLE, daWmKoopaCastle_c, 0);

daWmKoopaCastle_c::daWmKoopaCastle_c() : mReady(false) {}
daWmKoopaCastle_c::~daWmKoopaCastle_c() {}

int daWmKoopaCastle_c::create() {
    createModel();
    mClipSphere.set(mPos, 800.0f);
    calcModel();
    initState();
    return SUCCEEDED;
}

// @unofficial New types for two unnamed statics this TU's __sinit
// constructs (lbl_2_bss_10538, lbl_2_bss_10548) -- see execute()'s comment
// below for how the layout and values were derived. Kept file-local since
// no existing header declares a matching shape (grepped
// include/game/bases/, include/game/mLib/, and the landed WM siblings
// d_a_wm_smallcloud.cpp/grid.cpp/tower.cpp/cloud.cpp/peach_castle.cpp --
// none has a 2x-mVec3_c+bool or 4-int static of this shape).
namespace {
    struct KoopaShipPos_t {
        mVec3_c mPos1;
        mVec3_c mPos2;
        bool mDone;
        int mUnk1c; ///< @unofficial Never written by the target's __sinit
                     ///< (offsets 0x19-0x1f are untouched) -- needed purely
                     ///< to round sizeof() up to the target's 0x20, exact
                     ///< meaning (if any) unknown.

        KoopaShipPos_t() {
            if (!mDone) {
                mPos1 = mVec3_c(0.0f, 50.0f, -100.0f);
                mPos2 = mVec3_c(0.0f, 0.0f, -100.0f);
                mDone = true;
            }
        }
    };

    KoopaShipPos_t s_koopaShipPos;
}

int daWmKoopaCastle_c::execute() {
    static const ProcFunc Proc_tbl[PROC_COUNT] = {
        &daWmKoopaCastle_c::mode_exec
    };

    processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);
    (this->*Proc_tbl[mProcIdx])();

    daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    mModel.play();
    calcModel();

    // lbl_2_bss_10538 (16 bytes) and lbl_2_bss_10548 (32 bytes) are this
    // TU's two __sinit-constructed statics. lbl_2_bss_10538 turned out to
    // need NO new declaration: its only observed field (+0xc = int
    // dCsvData_c::c_START_ID) is already emitted automatically as
    // `dWmLib::c_StartPointKinokoHouseID` (d_wm_lib.hpp: `static int
    // c_StartPointKinokoHouseID = dCsvData_c::c_START_ID;`), pulled in
    // transitively the same way `dWmLib::sc_ForceList` already is -- both
    // are header-scope statics with dynamic (extern-const-dependent)
    // initialisers, so -ipa file keeps them regardless of whether this TU
    // reads them. check_sections confirmed it: the pre-existing, unnamed
    // 12-byte `@12806` (dWmLib::sc_ForceList's own array-registration
    // bookkeeping) plus `c_StartPointKinokoHouseID`'s 4 bytes sum to
    // exactly 0x10 at exactly the right relative offset (+0xc) -- adding a
    // SEPARATE declaration for lbl_2_bss_10538 double-counted it and
    // overshot .bss by 0x10 (tried and reverted).
    //
    // lbl_2_bss_10548 (KoopaShipPos_t below) genuinely is new. Its values
    // were read from the target's raw rodata bytes, not guessed: the
    // target's __sinit (fn_2_191C30) loads lbl_2_rodata_9860+0x1c/+0x30/
    // +0x34, and lbl_2_rodata_9860 is itself only a 4-byte symbol (800.0f,
    // used by mClipSphere.set in create()) -- the disassembler expresses
    // those loads as offsets FROM it because the surrounding float
    // constants are pooled without individual labels. Resolving the
    // addresses against the real rodata dump (rodata_8ef8.txt) gives
    // 0.0f / 50.0f / -100.0f -- plain compile-time constants, confirmed by
    // there being no `bl` in the guarded __sinit block (so NOT a runtime
    // CSV lookup, contrary to an earlier round's guess). mDone relies on
    // the SAME static-zero-init mechanism (starts false with no
    // constructor init-list entry, matching "always 0 on first run" from
    // the disassembly) rather than an explicit `: mDone(false)`, which the
    // compiler could constant-fold away and never emit the guard check
    // for at all. mUnk1c pads sizeof() to the target's 0x20 -- see its own
    // comment.
    mUnk26c = s_koopaShipPos.mPos1.x;
    mUnk270 = s_koopaShipPos.mPos1.y;
    mUnk274 = s_koopaShipPos.mPos1.z;
    mUnk278 = s_koopaShipPos.mPos2.x;
    mUnk27c = s_koopaShipPos.mPos2.y;
    mUnk280 = s_koopaShipPos.mPos2.z;

    return SUCCEEDED;
}

int daWmKoopaCastle_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmKoopaCastle_c::doDelete() {
    return SUCCEEDED;
}

// 6 differing (stack-slot-wall pattern, NOT the stack-slot wrapper defect --
// both create() calls already use the inline nullptr-forwarding wrapper).
// Residual is 3 by-value struct temps (outer resMdl-copy for mModel.create,
// and per-loop-iteration ResAnmChr-copy + resMdl-copy for mChrAnim[i].create)
// landing at target offsets 0x8/0xc/0x10(r1) but our offsets 0x10/0x8/0xc(r1)
// -- a pure 3-way stack-slot PERMUTATION, same bytes/registers otherwise.
// Tested and ruled out: (1) hoisting GetResAnmChr(...) into a named local
// before the loop's create() call -- zero effect; (2) moving the two static
// array declarations before vs after the resMdl/mModel.create statements --
// zero effect; (3) calling mModel.create via its 3-arg wrapper (one level
// deeper of inline nesting, matching anm_chr's double-wrapper depth) instead
// of the 4-arg one -- byte-identical output either way. None of these change
// a single emitted instruction, so this looks like the register/stack-slot
// numbering wall (not source-addressable), not a codegen-shape defect.
void daWmKoopaCastle_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("cobKoopaCastle", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("cobKoopaCastle");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    static const char *resAnmNames[ANIM_COUNT] = {
        "cobKoopaCastleOpen",
        "cobKoopaCastleClose",
        "cobKoopaCastleOut"
    };

    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_ONCE,
        m3d::FORWARD_ONCE,
        m3d::FORWARD_ONCE
    };

    for (int i = 0; i < ANIM_COUNT; i++) {
        mChrAnim[i].create(resMdl, mResFile.GetResAnmChr(resAnmNames[i]), &mAllocator);
        mChrAnim[i].mPlayMode = playModes[i];
        mChrAnim[i].setRate(0.0f);
        mChrAnim[i].setFrame(0.0f);
    }

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmKoopaCastle_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

// Matches fn_2_1915D0 exactly. Case LABEL declaration order (case 6, then
// 2/3/4, then 5/7, then 0/8, default EMPTY -- no constructCompanion() call)
// sets the target's physical body-block layout; the dispatch COMPARE order
// stayed compiler-chosen (ble-range for 2-4 first, then explicit 6/5/7/0/8).
// Member-init order also matters: mCompanionPlaced=false is written before
// csSeqMng/player are loaded, ahead of mCutscene=-1 and mIsOut=false.
void daWmKoopaCastle_c::initState() {
    mCompanionPlaced = false;
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    daWmPlayer_c *player = daWmPlayer_c::ms_instance;
    mCutscene = -1;
    mIsOut = false;

    if (IsCourseClear()) {
        int result = GetCurrentPlayResultStatus();
        switch (result) {
            case 6:
                if (IsCourseOtasukeClear()) {
                    mCutscene = 10;
                } else {
                    mCutscene = 11;
                }
                constructCompanion();
                break;

            case 2:
            case 3:
            case 4:
                mModel.setAnm(mChrAnim[ANM_OPEN]);
                mChrAnim[ANM_OPEN].setFrame(0.0f);
                mCutscene = 9;
                constructCompanion();
                break;

            case 5:
            case 7:
                mModel.setAnm(mChrAnim[ANM_OPEN]);
                mChrAnim[ANM_OPEN].setFrame(0.0f);
                mCutscene = 9;
                constructCompanion();
                break;

            case 0:
            case 8:
                if (!IsCourseOtasukeClear()) {
                    mIsOut = true;
                    mModel.setAnm(mChrAnim[ANM_OPEN]);
                    mChrAnim[ANM_OPEN].setFrame(mChrAnim[ANM_OPEN].mFrameMax - 1.0f);
                }
                constructCompanion();
                break;

            default:
                break;
        }
    } else if (GetCurrentPlayResultStatus() == 1) {
        mCutscene = 10;
    }

    if (mCutscene >= 0) {
        csSeqMng->FUN_801017c0((dCsSeqMng_c::CUTSCENE_e)mCutscene, this, player, 200);
    }

    init_exec();
}

// Matches fn_2_1917C0, the sibling of daWmCloud_c::init_exec().
void daWmKoopaCastle_c::init_exec() {
    mProcIdx = PROC_TYPE_EXEC;
}

// Matches fn_2_1917D0 exactly, the sibling of daWmCloud_c::mode_exec() -- an
// empty body (`blr`, no instructions).
void daWmKoopaCastle_c::mode_exec() {}

// Structure decoded directly from the raw target disassembly (fn_2_1917E0,
// params r3=this, r4=cutsceneCommandId kept live in r31, r5=isFirstFrame).
//
// "field_0x139" from the previous round's decode was a WRONG guess -- it is
// not a new koopa_castle member. Offset 0x139 is far BELOW where this
// class's own fields start (0x188), so it lives inside the base-class
// region; a member-offset probe (Probe : daWmKoopaCastle_c, taking
// &p->mIsCutEnd with p=(Probe*)0 and reading the immediate the compiler
// materialises) measured dWmDemoActor_c::mIsCutEnd at EXACTLY 0x139. Every
// `stb r0, 0x139(r30)` site below is `mIsCutEnd = true;`, confirmed against
// landed siblings (d_a_wm_cloud.cpp, d_a_wm_cannon.cpp, d_a_wm_peach.cpp,
// d_a_wm_peach_castle.cpp) which all write the same field the same way.
//
// SOLVED: the `isStaff() && GetClearStatus()==4 && (...)` else-branch
// (target instr 68-73: `lwz r12,0x60(r30); lwz r12,0x68(r12); bctrl` called
// with UNADJUSTED `this`) is a VIRTUAL CALL, not a data-member read.
// `this+0x60` is where THIS class's own vtable pointer lives -- confirmed
// directly from the constructor (fn_2_191100), which writes
// `lbl_2_data_4A370` (== `__vt__17daWmKoopaCastle_cFv`) to `this+0x60`
// right after the `dWmDemoActor_c` base-constructor call. `execute()`
// (fn_2_1912B0) independently confirms the slot numbering: it calls
// `processCutsceneCommand` the same way (`this+0x60` for the vtable, then
// vtable+0x60 for the function), so vtable SLOT 0x60/4=24 is
// processCutsceneCommand -- matching the vtable-24 note already on this
// function. `dWmDemoActor_c` declares `processCutsceneCommand`,
// `checkCutEnd`, `setCutEnd`, `clearCutEnd` in that order, so slot
// 0x68 (two slots after 0x60) is `setCutEnd()`. The else-branch is simply
// `setCutEnd();` -- a VIRTUAL call to the (unoverridden) base method,
// rather than the direct `mIsCutEnd = true;` used everywhere else in this
// function; MWCC did not devirtualize it here.
void daWmKoopaCastle_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        return;
    }

    if (isFirstFrame) {
        switch (cutsceneCommandId) {
        case 0x83:
            mReady = true;
            break;

        case 0x5f:
            if (isStaff() && GetClearStatus() == 4) {
                if (GetCurrentPlayResultStatus() == 4 || GetCurrentPlayResultStatus() == 7) {
                    mIsOut = false;
                    mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                    mModel.setAnm(mChrAnim[ANM_CLOSE]);
                    mChrAnim[ANM_CLOSE].setFrame(0.0f);
                    mChrAnim[ANM_CLOSE].setRate(1.0f);
                    dWmSeManager_c::m_pInstance->playSound(0x30, mPos, 1);
                } else {
                    setCutEnd();
                }
            } else {
                setCutEnd();
            }
            break;

        case dCsSeqMng_c::CUTSCENE_CMD_17:
            if (!mIsOut) {
                mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                mModel.setAnm(mChrAnim[ANM_OPEN]);
                mChrAnim[ANM_OPEN].setFrame(0.0f);
                mChrAnim[ANM_OPEN].setRate(1.0f);
                dWmSeManager_c::m_pInstance->playSound(0x2e, mPos, 1);
            }
            break;

        case dCsSeqMng_c::CUTSCENE_CMD_19:
            if (!mIsOut) {
                mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                mModel.setAnm(mChrAnim[ANM_OPEN]);
                mChrAnim[ANM_OPEN].setFrame(0.0f);
                mChrAnim[ANM_OPEN].setRate(1.0f);
                dWmSeManager_c::m_pInstance->playSound(0x2e, mPos, 1);
            }
            break;

        case dCsSeqMng_c::CUTSCENE_CMD_20:
            mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
            mModel.setAnm(mChrAnim[ANM_CLOSE]);
            mChrAnim[ANM_CLOSE].setFrame(0.0f);
            mChrAnim[ANM_CLOSE].setRate(1.0f);
            dWmSeManager_c::m_pInstance->playSound(0x30, mPos, 1);
            break;

        case 0x12:
            mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
            mModel.setAnm(mChrAnim[ANM_OUT]);
            mChrAnim[ANM_OUT].setFrame(0.0f);
            mChrAnim[ANM_OUT].setRate(1.0f);
            dWmSeManager_c::m_pInstance->playSound(0x2f, mPos, 1);
            break;

        default:
            break;
        }
    }

    switch (cutsceneCommandId) {
    case dCsSeqMng_c::CUTSCENE_CMD_17:
    case dCsSeqMng_c::CUTSCENE_CMD_19:
        if (!(mChrAnim[ANM_OPEN].isStop() || mIsOut)) {
            return;
        }
        mIsOut = true;
        mIsCutEnd = true;
        return;

    case dCsSeqMng_c::CUTSCENE_CMD_20:
        if (!mChrAnim[ANM_CLOSE].isStop()) {
            return;
        }
        mIsOut = false;
        mIsCutEnd = true;
        return;

    case 0x12:
        if (!mChrAnim[ANM_OUT].isStop()) {
            return;
        }
        mIsOut = true;
        mIsCutEnd = true;
        return;
    }

    mIsCutEnd = true;
}

// Matches fn_2_191B70. Guarded-once companion placement: a "koopa ship"-type
// actor (profile 0x29d) positioned at this model's "Koopa0" node.
void daWmKoopaCastle_c::constructCompanion() {
    if (mCompanionPlaced) {
        return;
    }
    mCompanionPlaced = true;

    const char *nodeName = "Koopa0";
    mVec3_c pos;
    pos = dWmLib::getModelNodePos(&mModel, nodeName);
    dWmActor_c::construct(0x29d, this, mParam, &pos, nullptr);
}

// Matches fn_2_191BF0 exactly -- the function daWmCourse_c depends on. Static:
// searches for ANOTHER daWmKoopaCastle_c instance (by this class's own profile
// ID, 0x29e) and returns its mReady flag, not this instance's own.
bool daWmKoopaCastle_c::isReady() {
    daWmKoopaCastle_c *other = (daWmKoopaCastle_c *)fManager_c::searchBaseByProfName(0x29e, nullptr);
    return other != nullptr ? other->mReady : false;
}
