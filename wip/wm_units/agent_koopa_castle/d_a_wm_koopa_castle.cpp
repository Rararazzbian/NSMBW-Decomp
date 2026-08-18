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
    // The guard is a SEPARATE file-scope static, not a struct member: the
    // target reads it off the ORIGINAL anchor (r30) and only computes the
    // struct's own derived pointer AFTER the guard test, whereas a member
    // guard has the draft hoist the derived pointer first and address the
    // guard through it too. Splitting it out reproduces that -- the
    // compiler has no reason to compute &s_koopaShipPos before it's
    // actually needed (inside the `if`), matching the target's ordering.
    // No initialiser on the guard either, same reasoning as before: an
    // explicit `= false` is something the compiler could prove and fold
    // the check away entirely; relying on static zero-init keeps the
    // guard real.
    //
    // DECLARATION ORDER matters here (as elsewhere in this project): the
    // guard's DEFINITION (the one that actually reserves .bss storage)
    // must come AFTER s_koopaShipPos, not before, or it lands in .bss
    // BEFORE the struct's own two mVec3_c fields instead of after them
    // (tried declaring+defining it first -- landed at +0x10 instead of
    // the target's +0x28; measured, not guessed). An out-of-line
    // constructor definition was ALSO tried, to let the guard be declared
    // after the class while still ordering its storage last -- that
    // regressed badly (21 -> 56 differing, the whole __sinit reshuffled),
    // so this uses a forward `extern` declaration instead, keeping the
    // constructor INLINE as before: the extern gives the inline method
    // early visibility of the name without affecting where the real
    // (non-extern) definition below reserves its storage.
    //
    // RESULT of the split: 22 -> 19 differing, and critically the guard
    // byte itself now sits at the CORRECT address (`0x28(r30)` in both --
    // confirmed instruction-for-instruction) instead of +0x18 through a
    // hoisted pointer. The remaining 19 were ALL the same single residual:
    // the target computes the struct's derived pointer (`addi r3,r30,
    // 0x10`) only AFTER the `bne` guard branch; the draft still computed
    // it (into r4) right after the guard load and BEFORE testing it --
    // an eagerly-materialised "this" versus a lazily-materialised one.
    // Three more source shapes were tried against exactly this residual,
    // none moved the count: (1) early-return (`if (done) return;` instead
    // of `if (!done) {...}`), (2) capturing `this` into a named local
    // AFTER the guard check, (3) building the two mVec3_c values as named
    // locals first and assigning them in, the same lever that closed
    // constructCompanion's stack-slot permutation. All three still
    // compiled to 19.
    //
    // SECOND ROUND -- 19 -> 13 via an inline-depth split, not a rephrasing.
    // The general "inline wrapper depth" rule found elsewhere this session
    // (createModel's 3-way stack-slot rotation, and the by-value-temporary
    // wall broken for wm_ghost/kinoko_base) also applies here: pushing the
    // GUARDED WRITES into their own inline member `doInit()` -- so the guard
    // test sits at depth 0 (directly in the ctor, calling doInit()) and the
    // writes sit at depth 1 (inside doInit()) instead of both at the same
    // depth -- took the residual from 19 to 13. A small depth-pair sweep
    // around that point (writes pushed to depth 2 via a further `assign()`
    // wrapper; the guard test itself routed through a depth-2 `isDone()`
    // accessor; doInit rewritten as a free function taking an explicit
    // `KoopaShipPos_t&`/operating on the global by name instead of implicit
    // `this`; the guarded writes split into two separate per-vector helper
    // calls; the writes expressed as direct scalar field stores instead of
    // `mPos1 = mVec3_c(...)`; naming `this` into a local first thing inside
    // the guarded block; and swapping `if (!guard)` for `if (guard == 0)`)
    // found only two other results: writes-at-depth-2 and the scalar-field
    // form both REGRESSED (19 and 33 respectively, the scalar form also
    // changing instruction count outright), and moving the guard-true
    // assignment before the writes regressed to 20 (already known). Every
    // other variant reproduced the identical 13-instruction residual below,
    // with the sole exception of the guard's own STORAGE TYPE (see below).
    //
    // The 13 that remain split into two groups, both inside the guarded
    // block, both variants of the same "materialise the pointer/register
    // only where it becomes live" pattern the target follows and the draft
    // does not:
    //   (a) The guard test itself: the target loads the byte then performs
    //       ONE `extsb. r0,r0` (sign-extend-and-record, folding the compare
    //       into the extend) where the draft does a separate `cmpwi r0,0`.
    //       This turned out to be the STORAGE TYPE of the guard, not the
    //       test's source spelling: `bool` (unsigned byte, C++ bool
    //       semantics) compiles the `cmpwi` form; changing ONLY the field's
    //       type to `s8` (signed char, matching the target's implied
    //       signed-byte test) reproduces the target's `extsb.` byte-for-byte
    //       -- confirmed in isolation. It does not by itself change the
    //       differing-count because of (b) below (an unrelated instruction
    //       still separates the two loads, shifting `extsb.` one slot out of
    //       phase), but it is a genuine, independently-verified fix and is
    //       kept.
    //   (b) The struct's derived pointer (`addi r3,r30,0x10`, i.e. &this
    //       object, which is `this` for `doInit()`/the ctor): the target
    //       computes it only AFTER the `bne`, and even then still writes
    //       the FIRST field (`mPos1.x`) through the ORIGINAL base register
    //       (`stfs f2,0x10(r30)`) rather than the freshly-computed pointer,
    //       switching to the pointer only from the second field write
    //       onward. The draft computes the pointer unconditionally at
    //       doInit()'s entry (before the guard test) and uses it uniformly
    //       for all six field writes. No source reshaping found in this
    //       round changed WHEN or THROUGH WHICH REGISTER that pointer gets
    //       materialised -- this looks like the same "temporary/pointer
    //       materialisation timing" wall diagnosed in the first round
    //       (see createModel's characterisation below), now narrowed to
    //       exactly this one register-allocation decision, and not
    //       addressable by further plain source rephrasing.
    //
    // Guard type wraps the byte WITH its trailing 7 unknown/unwritten
    // bytes (+0x29..+0x2f) in one object, rather than declaring the
    // padding as its own standalone global: an unreferenced standalone
    // global with no dynamic initialiser is exactly the kind of thing
    // `-ipa file` eliminates outright (tried it separately first -- .bss
    // stayed under by the padding's size, since nothing ever touches it).
    // Bundling it with the guard means the whole object is kept
    // because `mDone` IS referenced -- sizeof() doesn't shrink around an
    // unused sibling member the way an unused sibling GLOBAL can vanish.
    struct KoopaShipPosGuard_t {
        s8 mDone; ///< @unofficial Signed, not `bool`/`u8` -- matches the
                   ///< target's `extsb.` (sign-extend-and-test) guard read;
                   ///< see the long comment above. Same 0/1 values either
                   ///< way, only the storage type's signedness differs.
        u8 mPad[7]; ///< @unofficial Never written by the target's __sinit
                     ///< -- needed purely to round the claimed .bss span
                     ///< out to the target's 0x30 total, exact meaning (if
                     ///< any) unknown.
    };
    extern KoopaShipPosGuard_t s_koopaShipPosGuard;

    struct KoopaShipPos_t {
        mVec3_c mPos1;
        mVec3_c mPos2;

        // Depth-1 relative to the ctor (which stays depth-0, directly
        // calling this). See the long comment above -- this split is what
        // took the residual from 19 to 13; every depth variation tried
        // around it (writes pushed one level deeper still, the guard test
        // routed through its own accessor, free-function/explicit-self
        // forms, two separate per-vector helpers) reproduced the same 13
        // or regressed, so this is the shape being kept.
        void doInit() {
            if (!s_koopaShipPosGuard.mDone) {
                mPos1 = mVec3_c(0.0f, 50.0f, -100.0f);
                mPos2 = mVec3_c(0.0f, 0.0f, -100.0f);
                s_koopaShipPosGuard.mDone = true;
            }
        }

        KoopaShipPos_t() {
            doInit();
        }
    };

    KoopaShipPos_t s_koopaShipPos;
    KoopaShipPosGuard_t s_koopaShipPosGuard;
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
        mChrAnim[i].create(resMdl, mResFile.GetResAnmChr(resAnmNames[i]), &mAllocator, nullptr);
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
