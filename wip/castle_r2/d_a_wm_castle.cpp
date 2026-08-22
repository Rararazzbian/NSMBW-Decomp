#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_castle.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_wm_effect_manager.hpp>

// fn_80103420 is already in syms.txt (landed by the integrator from another unit's call site,
// confirmed against a live 5/5 tree). Signature corroborated there from ITS call site's argument
// registers, and independently checked here against THIS unit's own call site before use (see
// applyStopReaction below): r3=mgr, r4=kind, r5=&mModel, r6=name, r7/r8=trailing ints -- matches.
extern "C" void fn_80103420(dWmEffectManager_c *mgr, int kind, m3d::bmdl_c &model, const char *name, int, int);

// RESOLVED this task's round: __sinit now MATCHES (0 differing of 53). See PROBE F below for
// the mechanism (external trigger object, guard at the target's own measured offset, no
// registration) and this task's report for the full account, including the hard evidence that
// this object is initialised from `.ctors` (i.e. it IS `__sinit`, not a function-local static --
// see castle's own `fn_2_15FAE0`, referenced from `.ctors` at file offset 0x3c8, in
// bin/dtkspl/d_basesNP/obj/auto_fn_2_15FAE0_text.o).
//
// Kept below: the full probe history (A-F), left in place because it is what got here and
// because the eventual fix (PROBE F) only became findable by re-reading castle's AND
// koopa_castle's target `__sinit` byte-for-byte, which is worth keeping as a record of what did
// and did not work first. Target evidence: lbl_2_data_44010 (28 bytes, no relocations, read via
// dtk) is
//   0x42C80000 100.0f   0x3ECCCCCD 0.4f   [0.0f 100.0f 50.0f]=mVec3_c (dynamically initialised,
//   guarded by a byte in lbl_2_bss_FD48)   0x41200000 10.0f    0x00000000 0.0f
//
// PARKED at 16 differing (this shape), after four probes this round applying koopa_castle's
// guard/doInit()/depth-split lever tried to drop it further and ALL FOUR regressed. Recorded here
// so nobody re-tries them; __sinit's target disassembly for this object has NO
// `bl __register_global_object` / `__arraydtor` pair at all -- a plain byte-guard test
// (`lbz`/`extsb.`/`bne`) directly on a `.bss` byte, exactly koopa_castle's `KoopaShipPos_t` shape
// -- which is WHY these were worth trying; none of them reproduced it:
//   PROBE A -- give `KoopaShipStopConfig_t` its own constructor with a member-init-list
//   (`mUnk0(100.0f), ...`) calling a guarded `doInit()`. 28 differing, and regressed `createModel`
//   (0 -> 4) as a side effect. Once the type has ANY user-declared constructor, MWCC stops baking
//   the scalar fields (`mUnk0`/`mUnk4`/`mUnk14`/`mUnk18`) into the static `.data` image -- it
//   writes them at runtime instead, unconditionally, ahead of the guard branch. Structural, not
//   a register-allocation regression: the whole object moved from `.data` into `.bss`.
//   PROBE B -- drop the `[]` only, keep the plain aggregate initialiser, no constructor at all.
//   40 differing: without a hand-authored guard there is nothing to branch on, and without the
//   array there is no `__register_global_object` call either, so the compiler falls back to
//   unconditional runtime writes for the WHOLE object -- no guard, no branch, wrong shape.
//   PROBE C -- bare aggregate (no constructor), `mOffset` initialised via the empty-body
//   `mVec3_c()` default constructor (confirmed to bake the target's zeroed pre-guard image, since
//   a call that writes nothing has no observable effect to preserve), with the REAL values written
//   later by a SEPARATE trigger object's constructor reaching into
//   `sc_KoopaShipStopConfig.mOffset` from OUTSIDE the struct. 38 differing: the guard byte landed
//   at `.bss+0x18` (not the target's `+0x10`) and the stack frame grew to `0x40` (target `0x30`)
//   -- writing to another type's global through an external reference forces a
//   temporary-then-copy-assign rather than an in-place write, and that temporary's stack slots
//   aliased and reused the unrelated `sc_ForceList` staging slots earlier in the function.
//   PROBE D -- same split as C, but the guarded write moved back inside `KoopaShipStopConfig_t`
//   as an ORDINARY (non-constructor) member function `doInit()`, called on `this` the way
//   koopa_castle's `mPos1 = mVec3_c(...)` is (not through an external reference). Measured
//   BYTE-IDENTICAL to probe C, 38 differing, same `.bss+0x18` guard and `0x40` frame -- MWCC did
//   not inline the cross-object member call the way it inlines a constructor calling its own
//   class's `doInit()`, so the "member vs. external write" distinction that mattered for
//   koopa_castle's single-guarded-type shape does not carry over to this mixed
//   baked-scalars-plus-one-guarded-member shape.
// Conclusion: the koopa_castle lever needs the WHOLE object to be guard-driven to work; here only
// one field of five is, and every attempt to graft a guard onto part of an otherwise-aggregate
// object cost the `.data` bake, the frame size, or both. A non-const, namespace/file-scope array
// with an mVec3_c member (the shape below) remains the best found -- it reproduces `.data` and
// `.rodata` byte-for-byte and gets `__sinit` to 16 differing of 53. The guard's exact origin
// remains unexplained; `.bss` is 4 bytes over (two `__register_global_object` blocks vs. the
// target's apparent one, see this task's `check_sections.py --layout` output).
// PROBE E, this round: gave `mOffset` a hand-rolled trivial 3-float POD type instead of `mVec3_c`
// (whose real declaration in `include/game/mLib/m_vec.hpp` has a user-declared `~mVec3_c() {}` --
// confirmed by reading that header directly). CONFIRMED this is why the extra
// `bl __register_global_object` exists: with a POD offset, `KoopaShipStopConfig_t` becomes
// genuinely POD, the array needs no registration, and `__arraydtor`/`__dt__21KoopaShipStopConfig_tFv`
// both disappear -- `.text`'s second `bl __register_global_object` (the one the target does not
// have) is gone. But it overshoots: with NO non-trivial subobject left, MWCC now treats the whole
// `{0.0f, 100.0f, 50.0f}` initialiser as compile-time-constant and folds it entirely into `.data`
// at COMPILE time -- there is no runtime write AT ALL, not even an unconditional one, let alone the
// target's GUARDED one. That silently drops the `.rodata` float constants the runtime write used
// to load (`.rodata` UNDER by 4) and the guard-byte-and-friends `.bss` region the guard used to
// occupy (`.bss` UNDER by 8, worse than the `.data`-baked shape's OVER by 4). Net regression,
// reverted. The registration mechanism and the guarded-runtime-write are two SEPARATE, apparently
// independent things the target has one of each of (registration for the unrelated `sc_ForceList`,
// a guard for this object) and neither of my two known levers (array-of-mVec3_c vs. POD-array)
// produces "guard present, registration absent" at once -- see this task's report.
// @unofficial trivial POD standing in for mOffset's real type. Deliberately NOT mVec3_c: Probe E
// (see this task's report) proved mVec3_c's member here is what forces `KoopaShipStopConfig_t` --
// and therefore the whole `sc_KoopaShipStopConfig[]` array -- to be treated as non-trivial, which
// is what pulls in the SECOND `__register_global_object`/`__arraydtor` pair the target does not
// have. A trivial x/y/z POD lets the whole aggregate fold to a compile-time-constant `.data`
// image (matching the target's baked zeros), with no registration and no runtime write of its
// own -- exactly Probe E's result. This probe pairs that with Probe F's external trigger object
// so the GUARDED runtime write still happens, via plain float field assignment, which needs
// nothing from mOffset's type beyond public x/y/z.
struct Vec3Pod_t {
    float x, y, z;
};

struct KoopaShipStopConfig_t {
    float mUnk0;   ///< @unofficial 100.0f in the target.
    float mUnk4;    ///< @unofficial 0.4f in the target.
    Vec3Pod_t mOffset; ///< @unofficial {0.0f, 100.0f, 50.0f} in the target -- the guarded part.
    float mUnk14;     ///< @unofficial 10.0f in the target.
    float mUnk18;      ///< @unofficial 0.0f in the target.
};

// ACTOR_PROFILE must sit HERE, ahead of sc_KoopaShipStopConfig. `.data` object order is set by
// declaration order, and retail has g_profile_WM_CASTLE at 0x44004 with sc_KoopaShipStopConfig
// following it at 0x44010 -- the address the target's own getKoopaShipStopPos relocation names.
// With the profile declared after the config (as this draft had it) both objects are emitted,
// both sections are the right SIZE, every function still matches, and the two objects are simply
// swapped: exactly the silent failure class that cost d_a_wm_smallcloud a landing.
ACTOR_PROFILE(WM_CASTLE, daWmCastle_c, 0);

static KoopaShipStopConfig_t sc_KoopaShipStopConfig[] = {
    { 100.0f, 0.4f, {0.0f, 0.0f, 0.0f}, 10.0f, 0.0f }
};

// PROBE F, this task's round. Ground truth re-read directly from bin/dtkspl/d_basesNP/obj/
// auto_fn_2_15FAE0_text.o (castle's real __sinit, referenced from .ctors -- confirmed this is
// NOT a function-local static: it is called exactly once, automatically, from the constructor
// table, never from any runtime call site). That disassembly settles precisely what Probes A-E
// only approximated:
//   - The guard byte is `lbz`/`extsb.`/`bne` against `lbl_2_bss_FD48+0x10`, and set with
//     `stb r0, 0x10(r30)` afterward -- a plain hand-checked flag, not compiler guard-variable
//     codegen (which would show a `__sinit_...`-suffixed guard symbol and register differently).
//   - `lbl_2_bss_FD48` is EXACTLY 0x18 bytes (confirmed against target_bss.txt's own .obj
//     bound) and nothing else in this TU's compiled text -- not the constructor, not
//     getKoopaShipStopPos, not any other function -- reads or writes bytes 0x0-0xb or 0x11-0x17
//     of it. Only two bytes are ever touched: `+0xc` (an int, loaded unconditionally from
//     `dCsvData_c::c_START_ID` on every __sinit run, BEFORE the guard branch) and `+0x10` (the
//     guard byte itself).
//   - The guarded write lands in a SEPARATE object, `lbl_2_data_44010` (castle's own
//     `sc_KoopaShipStopConfig`), via three plain `stfs` stores -- never a `bl __ct__6mVec3_c...`.
//     No `bl __register_global_object` appears anywhere near this write.
// Probes C and D already tried this general shape (external trigger / member `doInit()`) and
// landed at 38 differing with the guard at `.bss+0x18` and frame `0x40`, both 8/16 bytes over
// this round's directly-measured target (`+0x10`, frame `0x30`).
//
// FIRST cut of this probe gave the trigger its OWN `mStartID(dCsvData_c::c_START_ID)` field --
// wrong: it compiled to a SECOND, redundant `c_START_ID` load-and-store next to the shared
// header's OWN `dWmLib::c_StartPointKinokoHouseID = dCsvData_c::c_START_ID;` (already declared
// above, in d_wm_lib.hpp), which is ALSO a namespace-scope header static with a non-constant
// (extern-loaded) initialiser and therefore ALSO gets a per-TU dynamic-init entry in THIS unit's
// own __sinit. koopa_castle's independently-read ground truth
// (bin/dtkspl/d_basesNP/obj/auto_fn_2_191C30_text.o) has the IDENTICAL
// `lis/lwz c_START_ID; ...; stw r3, 0xc(r30)` sequence at the same relative position, which is
// only explicable if both units are compiling the SAME shared header static, not a
// castle-local field -- confirming the target's lone `.bss+0xc` write is
// `c_StartPointKinokoHouseID`'s own initialiser, not a member of any castle-declared struct.
// Removing `mStartID` (letting the existing header static supply that write for free) and
// leaving the trigger as pure guard state is what closes the offset gap below.
struct KoopaShipStopTrigger_t {
    s8 mDone; ///< @unofficial the guard byte at lbl_2_bss_FD48+0x10 -- s8, not bool: the
              ///< target tests it with `extsb.` (sign-extend + implicit compare), which is
              ///< NOT what `bool mDone` compiled to here (that gave `lbz`+`cmpwi`, one
              ///< instruction longer). koopa_castle's independently-read ground truth
              ///< (fn_2_191C30) shows the identical `extsb.` idiom for its own guard byte.
    u8 pad_unofficial[7]; ///< @unofficial trailing bytes of lbl_2_bss_FD48 (0x11-0x17) that no
                           ///< function in this TU reads or writes. check_sections.py reports
                           ///< the object 7 bytes short of the map's single 0x18 lbl_2_bss_FD48
                           ///< symbol without this -- see this task's report for what is and
                           ///< is not established about their true size/content.

    KoopaShipStopTrigger_t() {
        if (!mDone) {
            // Target's own instructions 40-45 stage the three floats through r1+0x8/0xc/0x10
            // BEFORE the real store (the mVec3_c::Zero idiom AGENT_CONTEXT.md records). A
            // Vec3Pod_t local built via BRACE-INIT (`{0.0f, 100.0f, 50.0f}`) reproduced that
            // staging but REGRESSED two previously matching functions (checkCourseResult,
            // processCutsceneCommand) -- the aggregate literal apparently perturbs this TU's
            // .rodata pool ordering. Building the SAME local via individual field assignment
            // (below) gives the identical staged-copy codegen without touching the pool --
            // this is the shape that closed __sinit to a MATCH.
            Vec3Pod_t offset;
            offset.x = 0.0f;
            offset.y = 100.0f;
            offset.z = 50.0f;
            sc_KoopaShipStopConfig[0].mOffset = offset;
            mDone = true;
        }
    }
};

static KoopaShipStopTrigger_t sc_KoopaShipStopTrigger;

daWmCastle_c::daWmCastle_c() : m_2b4(0) {}
daWmCastle_c::~daWmCastle_c() {}

int daWmCastle_c::create() {
    createModel();
    mClipSphere.set(mPos, 250.0f);
    calcModel();
    checkCourseResult();
    return SUCCEEDED;
}

// Proc_tbl's definition belongs HERE, after create(), not at the top of the file. `.rodata` is
// laid out in emission order and a function's literal-pool entries are emitted when that
// function is compiled, so the 250.0f in create() above lands at 0x86e8 and Proc_tbl at 0x86ec
// -- which is retail's order. Defined before create(), both objects are still emitted at the
// right sizes and the section total is identical; they are just the wrong way round.
const daWmCastle_c::ProcFunc daWmCastle_c::Proc_tbl[PROC_COUNT] = {
    &daWmCastle_c::mode_exec
};

int daWmCastle_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    (this->*Proc_tbl[mCurrProc])();

    daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    mModel.play();
    calcModel();

    return SUCCEEDED;
}

int daWmCastle_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmCastle_c::doDelete() {
    return SUCCEEDED;
}

void daWmCastle_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);
    mResFile = dResMng_c::m_instance->getRes("cobCastle", "g3d/model.brres");

    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("cobCastle");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    static const char *resAnmNames[ANIM_COUNT] = {
        "cobCastleOpen",
        "cobCastleClose",
        "cobCastleOut",
        "cobCastleShake"
    };

    // FORWARD_ONCE (=1), not FORWARD_LOOP (=0). Read straight out of retail:
    // `lbl_2_rodata_86F8` is four `0x00000001` words. This is invisible to every per-function
    // check -- `createModel` is byte-identical either way, because the table is reached through
    // a relocation whose address field is zeroed on both sides, and check_sections.py only sees
    // that the object is the right SIZE. Caught by byte-comparing .rodata against the target
    // (wip/castle_r2/datacheck.py); the draft had all four as 0.
    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_ONCE, m3d::FORWARD_ONCE, m3d::FORWARD_ONCE, m3d::FORWARD_ONCE
    };

    for (int i = 0; i < ANIM_COUNT; i++) {
        nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(resAnmNames[i]);
        mChrAnim[i].create(resMdl, resAnmChr, &mAllocator, nullptr);
        mChrAnim[i].mPlayMode = playModes[i];
        mChrAnim[i].setRate(0.0f);
        mChrAnim[i].setFrame(0.0f);
    }

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmCastle_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

// Best effort: the tail's two child-actor spawns (WM_CANNON near a special-clear world, and the
// unconditional WM_CANNON spawn when the course is not clear) pass a POSITION the target leaves
// genuinely uninitialized on the stack (no store to it is visible before the `bl construct`), so
// this is left as an uninitialized local rather than invented. Not verified byte-exact.
void daWmCastle_c::checkCourseResult() {
    m_2b0 = false;
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    daWmPlayer_c *player = daWmPlayer_c::ms_instance;
    mCutscene = -1; // dCsSeqMng_c::CUTSCENE_e has no "none" enumerator in this project's header yet
    m_2a0 = false;

    if (IsCourseClear()) {
        int status = GetCurrentPlayResultStatus();
        switch (status) {
            case 6:
                // SMC_DEMO_CASTLE_FAIL2 (11) has no header enumerator; SMC_DEMO_CASTLE_FAIL is 10.
                if (!IsCourseOtasukeClear()) {
                    mCutscene = 11;
                } else {
                    mCutscene = dCsSeqMng_c::SMC_DEMO_CASTLE_FAIL;
                }
                spawnKoopaNodeEffect();
                break;
            case 2:
            case 3:
            case 4:
                mModel.setAnm(mChrAnim[ANIM_OPEN]);
                mChrAnim[ANIM_OPEN].setFrame(0.0f);
                if (IsCourseFirstClear()) {
                    if (dWmLib::hasKoopaShipStop()) {
                        if (dWmLib::isKoopaShipOnCurrentWorld()) {
                            mCutscene = dCsSeqMng_c::SMC_DEMO_W3_CASTLE_CLR;
                        } else {
                            mCutscene = dCsSeqMng_c::SMC_DEMO_CASTLE_CLR;
                        }
                    } else {
                        mCutscene = dCsSeqMng_c::SMC_DEMO_W1_CASTLE_CLR;
                    }
                } else {
                    mCutscene = dCsSeqMng_c::SMC_DEMO_CASTLE_CLR;
                }
                spawnKoopaNodeEffect();
                break;
            case 5:
            case 7:
                mModel.setAnm(mChrAnim[ANIM_OPEN]);
                mChrAnim[ANIM_OPEN].setFrame(0.0f);
                mCutscene = dCsSeqMng_c::SMC_DEMO_CASTLE_CLR;
                spawnKoopaNodeEffect();
                break;
            case 0:
            case 8:
                if (!IsCourseOtasukeClear()) {
                    m_2a0 = true;
                    mModel.setAnm(mChrAnim[ANIM_OPEN]);
                    mChrAnim[ANIM_OPEN].setFrame(mChrAnim[ANIM_OPEN].mFrameMax - 1.0f);
                    spawnKoopaNodeEffect();
                }
                break;
        }
    } else if (GetCurrentPlayResultStatus() == 1) {
        mCutscene = dCsSeqMng_c::SMC_DEMO_CASTLE_FAIL;
    }

    if (mCutscene >= 0) {
        csSeqMng->FUN_801017c0((dCsSeqMng_c::CUTSCENE_e) mCutscene, this, player, 200);
    }

    // CONFIRMED: one function-wide local reused across both distant call sites (rather than two
    // block-scoped locals of the same name) is what makes the target allocate ONE stack slot for
    // both instead of two -- closed the frame from -0x40 to the target's -0x30 and took this
    // function's differing-instruction count from 25 to 4 (see this task's report). A prior round
    // tried narrowing each temporary into its OWN disjoint brace scope and that did nothing; the
    // lever that actually matters is a single declaration whose lifetime spans both use sites, not
    // scope-narrowing.
    mVec3_c pos; // see function comment -- uninitialized in the target

    if (!dWmLib::isSpecialWorld() && dWmLib::isKoopaShipOnCurrentWorld()) {
        if (mCutscene == dCsSeqMng_c::SMC_DEMO_W1_CASTLE_CLR || mCutscene == dCsSeqMng_c::SMC_DEMO_W3_CASTLE_CLR) {
            construct(fProfile::WM_KOOPASHIP, this, 2, &pos, nullptr);

            if (mCutscene == dCsSeqMng_c::SMC_DEMO_W3_CASTLE_CLR) {
                // CONFIRMED: this is the third arrangement, and the one that matches. The direct
                // constructor call `mVec3_c pos2(mPos.x, mPos.y, mPos.z - 100.0f)` evaluates right
                // to left (z, y, x -- matching target instruction order) but MWCC allocates f2/f3 to
                // y/x in DESCENDING order (first-evaluated gets the higher-numbered register),
                // whereas the target allocates them ASCENDING (first-evaluated gets f2, next gets
                // f3) -- a pure register-numbering swap, 4 differing. Staging ALL THREE through
                // named locals declared x2,y2,z2 (natural order) flips to ascending allocation but
                // ALSO flips the evaluation order to x,y,z (mismatching target's z,y,x), still 4
                // differing, in the opposite pair of instructions -- this is the "opposite direction"
                // swap. Declaring the locals in z2,y2,x2 order (matching the target's own evaluation
                // order) while still PASSING them to the constructor in natural x2,y2,z2 order gets
                // BOTH right at once: evaluation order z,y,x (from declaration order) and ascending
                // f2/f3 allocation (from routing through already-live named locals rather than raw
                // member expressions) -- byte-exact.
                float z2 = mPos.z - 100.0f;
                float y2 = mPos.y;
                float x2 = mPos.x;
                mVec3_c pos2(x2, y2, z2);
                construct(fProfile::WM_KOOPAJR, this, 0, &pos2, nullptr);
            }
        }
    }

    if (!IsCourseClear()) {
        pos = mVec3_c::Zero;
        construct(fProfile::WM_KOOPASHIP, this, 1, &pos, nullptr);
    }

    resetReaction();
}

void daWmCastle_c::resetReaction() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmCastle_c::mode_exec() {}

// GIANT function (~250 instructions), left for last per this task's brief. Best-effort
// reconstruction from codegen evidence; NOT verified byte-exact. CUTSCENE_CMD_e is missing the
// 18 and 95 enumerators the target switches on (see this task's report for the proposed diff).
void daWmCastle_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        return;
    }

    if (!isStaff()) {
        mIsCutEnd = true;
        return;
    }

    if (isFirstFrame) {
        switch (cutsceneCommandId) {
            case 0x5f: // CUTSCENE_CMD_95, no header enumerator yet
                if (GetClearStatus() == 4) {
                    if (GetCurrentPlayResultStatus() == 4 || GetCurrentPlayResultStatus() == 7) {
                        m_2a0 = false;
                        mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                        mModel.setAnm(mChrAnim[ANIM_CLOSE]);
                        mChrAnim[ANIM_CLOSE].setFrame(0.0f);
                        mChrAnim[ANIM_CLOSE].setRate(1.0f);
                        dWmSeManager_c::m_pInstance->playSound(0x22, mPos, 1);
                    } else {
                        setCutEnd();
                    }
                } else {
                    setCutEnd();
                }
                break;
            case dCsSeqMng_c::CUTSCENE_CMD_17: {
                fBase_c *found = fManager_c::searchBaseByProfName(fProfile::WM_KOOPASHIP, nullptr);
                if (found == nullptr) {
                    m_2b4 = 0x3c;
                } else {
                    m_2b4 = 1;
                }
                break;
            }
            case dCsSeqMng_c::CUTSCENE_CMD_19:
                if (!m_2a0) {
                    mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                    mModel.setAnm(mChrAnim[ANIM_OPEN]);
                    mChrAnim[ANIM_OPEN].setFrame(0.0f);
                    mChrAnim[ANIM_OPEN].setRate(2.4f);
                    dWmSeManager_c::m_pInstance->playSound(0x20, mPos, 1);
                }
                break;
            case dCsSeqMng_c::CUTSCENE_CMD_20:
                mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                mModel.setAnm(mChrAnim[ANIM_CLOSE]);
                mChrAnim[ANIM_CLOSE].setFrame(0.0f);
                mChrAnim[ANIM_CLOSE].setRate(1.0f);
                dWmSeManager_c::m_pInstance->playSound(0x22, mPos, 1);
                break;
            case 0x12: // CUTSCENE_CMD_18, no header enumerator yet
                mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                mModel.setAnm(mChrAnim[ANIM_OUT]);
                mChrAnim[ANIM_OUT].setFrame(0.0f);
                mChrAnim[ANIM_OUT].setRate(1.0f);
                dWmSeManager_c::m_pInstance->playSound(0x21, mPos, 1);
                break;
            default:
                break;
        }
    }

    switch (cutsceneCommandId) {
        case dCsSeqMng_c::CUTSCENE_CMD_17:
            if (m_2b4 == 1) {
                if (!m_2a0) {
                    mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                    mModel.setAnm(mChrAnim[ANIM_OPEN]);
                    mChrAnim[ANIM_OPEN].setFrame(0.0f);
                    mChrAnim[ANIM_OPEN].setRate(2.4f);
                    dWmSeManager_c::m_pInstance->playSound(0x20, mPos, 1);
                }
                m_2b4--;
            } else if (m_2b4 > 0) {
                m_2b4--;
            } else if (mChrAnim[ANIM_OPEN].isStop() || m_2a0) {
                m_2a0 = true;
                mIsCutEnd = true;
            }
            break;
        case dCsSeqMng_c::CUTSCENE_CMD_19:
            if (mChrAnim[ANIM_OPEN].isStop() || m_2a0) {
                m_2a0 = true;
                mIsCutEnd = true;
            }
            break;
        case dCsSeqMng_c::CUTSCENE_CMD_20:
            if (mChrAnim[ANIM_CLOSE].isStop()) {
                m_2a0 = false;
                mIsCutEnd = true;
            }
            break;
        case 0x12: // CUTSCENE_CMD_18
            m_2a0 = true;
            mIsCutEnd = true;
            break;
        default:
            mIsCutEnd = true;
            break;
    }
}

void daWmCastle_c::spawnKoopaNodeEffect() {
    if (!m_2b0) {
        m_2b0 = true;
        getKoopaPos(mKoopaSpawnPos);
        construct(fProfile::WM_SURRENDER, this, mParam, &mKoopaSpawnPos, nullptr);
    }
}

// Previously not authored on the reading that fn_2_15F950's `this` (the fBase_c* returned by
// searchBaseByProfName) meant it belonged to a different, undecompiled class ("WM_ANTLION_MNG").
// That profile-ID read was wrong: fProfile::WM_ANTLION_MNG is 0x271, but the target's immediate
// is `li r3, 0x272`, which is fProfile::WM_CASTLE (compiler-verified, not counted from source --
// see this task's report). searchBaseByProfName is searching for ANOTHER daWmCastle_c, and the
// symbol binding confirms it: both this function and fn_2_15F950 are GLOBAL (not weak) in the
// target, which rules out an inline-in-another-class's-header definition (inline is always
// weak) and is consistent with these being ordinary daWmCastle_c members after all.
void daWmCastle_c::TriggerCastleStopReaction(float rate, float frame) {
    daWmCastle_c *castle = (daWmCastle_c *) fManager_c::searchBaseByProfName(fProfile::WM_CASTLE, nullptr);
    if (castle != nullptr) {
        castle->applyStopReaction(rate, frame);
    }
}

void daWmCastle_c::applyStopReaction(float rate, float frame) {
    if (mChrAnim[ANIM_SHAKE].isStop() || mChrAnim[ANIM_SHAKE].getRate() == 0.0f) {
        mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
        mModel.setAnm(mChrAnim[ANIM_SHAKE]);
        mChrAnim[ANIM_SHAKE].setRate(rate);
        mChrAnim[ANIM_SHAKE].setFrame(frame);
        dWmSeManager_c::m_pInstance->playSound(0x4a, mPos, 1);
        fn_80103420(dWmEffectManager_c::m_pInstance, 0x29, mModel, "cobCastle", 0, 0);
    }
}

bool daWmCastle_c::getKoopaPos(mVec3_c &out) const {
    out = dWmLib::GetModelNodePos(&mModel, "Koopa");
    return true;
}

// CLOSED (0 differing). The whole residual was `const` on the member function, and NOTHING in
// the body. Do not re-add it: `const` here costs exactly 6 of these 15 instructions.
//
// Mechanism. A by-value `mVec3_c` return is written through the hidden result pointer in `r3`.
// When `this` is NON-const, MWCC's scheduler cannot prove `*r3` does not alias `*this`, so it
// keeps every load ahead of every store and emits the tail in its natural order: the third
// component's member load before its offset load, then the three stores contiguously as
// y, x, z. Declaring the function `const` licenses the scheduler to move the first store up
// into the middle of the arithmetic; it then hoists `stfs f4, 0x8(r3)` three slots and swaps
// the two x loads, which is precisely the 6-instruction residual this function was parked on.
//
// Confirmed against the matched corpus, not just by A/B compile -- and the corpus predicts the
// answer on its own. Every byte-exact function in the tree that returns a 3-float aggregate
// through `r3` splits the same way by constness:
//   * `dBaseActor_c::getCenterPos() const` (d_base_actor.cpp) -- SAME arithmetic as this
//     function, `return mPos + mCenterOffs;`, and it has the HOISTED store (order 8,4,0 split by
//     an `fadds`), i.e. exactly what this draft produced while it was declared `const`.
//   * `daWmAntlion_c::calcBlowOffPos(f32)` and `daWmAntlion_c::getPointOffset(int)` (this very
//     module) and their `dWmEnemy_c` twins -- all NON-const, all with the contiguous
//     4,0,8 tail this target has.
// So the store-tail shape reads the constness of the member function straight off the
// disassembly. `getPointOffset`/`calcBlowOffPos` are non-const in the landed sources too.
//
// Ruled out on the way, all measured this round, all byte-identical to the parked shape unless
// noted -- recorded so nobody spends a round on them again:
//   - all six x/y/z COMPUTE orders (6/8/8/10/10/10) and all six field-STORE orders (all 6);
//   - `return mVec3_c(x,y,z)` vs a named `result` with field assignment vs `result.set(x,y,z)`
//     vs `mVec3_c result(...)` -- RVO makes all four identical;
//   - `mPos + offset` through `mVec3_c::operator+` (reinterpret_cast on the POD) -- identical;
//   - all EIGHT per-component addend-order flips -- they permute which of f0/f1 holds which
//     operand and never once move the two load instructions or the store;
//   - def-point levers 11/12/13 on any leaf (`f32 mx = mPos.x;` etc., in five placements) --
//     11-14 differing: the def-pointed value takes f4 and shifts every other FPR down one,
//     exactly the "a def-point is not free" rule in AGENT_CONTEXT.md;
//   - the offset lvalue as a reference / plain pointer / element pointer / bare `float*` /
//     non-const reference, and `mPos` via a `const mVec3_c &` or its `EGG::Vector3f` base --
//     all identical;
//   - compiler flags, as a diagnostic only: `-O4`, `-O4,s` and `-opt schedule/nopeephole/
//     nointrinsics` all give the identical 6, so the residual was never a flags artefact.
//     `-opt noschedule` reproduces this target's TAIL exactly, which is what identified the
//     scheduler as the agent and led to the constness test.
mVec3_c daWmCastle_c::getKoopaShipStopPos() {
    const Vec3Pod_t &offset = sc_KoopaShipStopConfig[0].mOffset;
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    float x = mPos.x + offset.x;
    mVec3_c result;
    result.y = y;
    result.x = x;
    result.z = z;
    return result;
}
