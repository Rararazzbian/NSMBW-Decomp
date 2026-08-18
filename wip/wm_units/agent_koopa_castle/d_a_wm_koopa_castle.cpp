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

int daWmKoopaCastle_c::execute() {
    static const ProcFunc Proc_tbl[PROC_COUNT] = {
        &daWmKoopaCastle_c::mode_exec
    };

    processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);
    (this->*Proc_tbl[mProcIdx])();

    daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    mModel.play();
    calcModel();

    // TODO: not yet matched -- target reads 6 floats from lbl_2_bss_10548
    // into mUnk26c..mUnk280 here every frame (fn_2_1912B0 tail, 20 instrs).
    //
    // CORRECTION to the previous round's read: lbl_2_bss_10538 and
    // lbl_2_bss_10548 are TWO SEPARATE bss symbols, not one 0x30-byte
    // object read at a +0x10 offset -- confirmed from bss_10040.txt (the
    // target's .bss map): `lbl_2_bss_10538, size 0x10` immediately
    // followed by `lbl_2_bss_10548, size 0x20`, and execute() itself loads
    // `lbl_2_bss_10548@ha/@l` DIRECTLY (see fn_2_1912B0), never
    // bss_10538+0x10. The apparent "+0x10 offset" in __sinit (fn_2_191C30)
    // is `addi r3, r30, 0x10` where r30 = &lbl_2_bss_10538 -- that IS the
    // address of lbl_2_bss_10548, just computed relative to its neighbour
    // instead of via its own lis/ha (the two are laid out adjacently).
    //
    // Re-mapping the __sinit writes onto the two REAL objects:
    //   * lbl_2_bss_10538 (16 bytes): ONE write observed, +0xc = int
    //     dCsvData_c::c_START_ID, unconditional (not guarded). +0x0..+0xb
    //     are never written by this TU -- unknown, left as .bss zero.
    //   * lbl_2_bss_10548 (32 bytes): +0x0/+0x4/+0x8 = 3 floats loaded
    //     DIRECTLY from lbl_2_rodata_9860+0x1c/+0x30/+0x34 (plain constant
    //     loads, NO function call in the guarded block -- this is NOT a
    //     runtime CSV lookup, contrary to the previous round's guess).
    //     +0xc/+0x10 duplicate the +0x1c-rodata value a second and third
    //     time, +0x14 duplicates the +0x34-rodata value a second time.
    //     +0x18 = a guard byte, tested via extsb./bne BEFORE the writes and
    //     set to 1 after -- always 0 on first run since .bss zero-inits,
    //     so (as before) the guard is never observed skipping anything.
    //     +0x19..+0x1f (7 bytes) are never written -- likely alignment
    //     padding, but could hide another field.
    //   execute()'s 6-float copy is exactly lbl_2_bss_10548+0x0..+0x18.
    //
    // Also confirmed: lbl_2_data_4A2D0 (.data, size 0x24, dtor
    // __dt__Q26dWmLib19ForceInCourseList_tFv via array-dtor fn_2_191D20)
    // IS dWmLib::sc_ForceList (d_wm_lib.hpp) -- its fields (c_CASTLE_ID at
    // +0xc, 3 floats at +0x18/+0x1c/+0x20 from lbl_2_rodata_9860+
    // 0x24/0x28/0x2c) match the header's existing initialiser
    // {WORLD_7,"F7C0",WORLD_7,c_CASTLE_ID,4,"W7C0",mVec3_c(2160,-30,-478)}
    // exactly -- getting this TU to odr-use sc_ForceList (not yet done
    // below) should emit it automatically, no new type needed for THAT
    // part.
    //
    // Still open: no existing header declares a type shaped like
    // lbl_2_bss_10538/10548. Given the guard is on the SECOND object only
    // and both are written by the SAME __sinit call with no intervening
    // function call, the likeliest shape is two plain file-scope statics
    // (not a class with a lazy-init member function) -- e.g. a bare
    // `int s_level = dCsvData_c::c_START_ID;` plus a guarded
    // `struct { mVec3_c a, b; bool done; } s_pos;` pair initialised by a
    // single free function called once at namespace scope. NOT attempted
    // this round: getting __sinit itself byte-exact (it is not one of the
    // 16 counted functions, but execute() needs the right symbols to
    // exist) requires iterating on that free function's exact shape, which
    // needs a fresh compile/diff cycle against fn_2_191C30 -- next round.

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
