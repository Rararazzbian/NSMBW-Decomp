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

// EXPERIMENTAL, castle-local (moved out of the shared d_wm_lib.hpp shadow -- see this task's
// report). Target evidence: lbl_2_data_44010 (28 bytes, no relocations, read via dtk) is
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
struct KoopaShipStopConfig_t {
    float mUnk0;   ///< @unofficial 100.0f in the target.
    float mUnk4;    ///< @unofficial 0.4f in the target.
    mVec3_c mOffset; ///< @unofficial {0.0f, 100.0f, 50.0f} in the target -- the guarded part.
    float mUnk14;     ///< @unofficial 10.0f in the target.
    float mUnk18;      ///< @unofficial 0.0f in the target.
};

static KoopaShipStopConfig_t sc_KoopaShipStopConfig[] = {
    { 100.0f, 0.4f, mVec3_c(0.0f, 100.0f, 50.0f), 10.0f, 0.0f }
};

ACTOR_PROFILE(WM_CASTLE, daWmCastle_c, 0);

const daWmCastle_c::ProcFunc daWmCastle_c::Proc_tbl[PROC_COUNT] = {
    &daWmCastle_c::mode_exec
};

daWmCastle_c::daWmCastle_c() : m_2b4(0) {}
daWmCastle_c::~daWmCastle_c() {}

int daWmCastle_c::create() {
    createModel();
    mClipSphere.set(mPos, 250.0f);
    calcModel();
    checkCourseResult();
    return SUCCEEDED;
}

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

    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_LOOP, m3d::FORWARD_LOOP, m3d::FORWARD_LOOP, m3d::FORWARD_LOOP
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

// PARKED at 6 differing (this shape) after this task's round of probes, none of which moved it.
// Two defects, and they may not be independent: (1) computing x's operands, the target loads
// `mPos.x` THEN `offset.x` (register f1 then reuses f0); this draft loads `offset.x` THEN
// `mPos.x` (register f0 reused first, then f1) -- backwards only for x; z and y both load their
// `mPos.*` member before their `offset.*` member, matching the target's own pattern for z/y, so
// x is the outlier. (2) the target defers ALL THREE field stores to after all three adds are
// computed (store order y, x, z); this draft computes z and y, STORES z immediately, THEN
// computes x and stores y, x -- an extra store is scheduled early relative to the target.
//
// Per this task's brief, reordering the x/y/z LOCAL DECLARATIONS was not retried here (two
// orderings already measured worse, 13 and 7 against this 6). What WAS tried this round, all
// giving byte-identical output to this shape (no change) unless noted:
//   - swapping the STORE statement order (result.x=x;result.y=y;result.z=z, and z,y,x) -- MWCC
//     schedules these independent stores by its own readiness heuristic regardless of the written
//     order; no combination of store order changed a single byte.
//   - dropping the `offset` reference and reading `sc_KoopaShipStopConfig[0].mOffset.*` fresh at
//     each use -- CSE's the address back to one `lis/addi` either way, byte-identical.
//   - `return mVec3_c(x, y, z);` in place of a named `result` local with field assignments --
//     byte-identical (MWCC already applies RVO to the named-local form).
//   - collapsing straight to `return mVec3_c(mPos.x + offset.x, mPos.y + offset.y, mPos.z +
//     offset.z);` with no locals at all -- still evaluates z, y, x (right-to-left) and is
//     byte-identical to the baseline shape above.
//   - flipping x's addend order to `offset.x + mPos.x` -- DID change the instructions (f0/f1
//     swap in the opposite direction, still register f0/f1 misassigned) but not the differing
//     COUNT: still 6, just a different pair of wrong instructions. Reverted.
// The lever that closed the analogous f2/f3 swap in checkCourseResult (decoupling local
// DECLARATION order from constructor ARGUMENT order) was tried here too, in the no-locals and
// direct-mVec3_c-construction forms above, without effect -- this function returns through a
// hidden result pointer (r3) rather than storing into a stack temporary later passed by address,
// and that ABI difference may be why the lever does not transfer.
mVec3_c daWmCastle_c::getKoopaShipStopPos() const {
    const mVec3_c &offset = sc_KoopaShipStopConfig[0].mOffset;
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    float x = mPos.x + offset.x;
    mVec3_c result;
    result.y = y;
    result.x = x;
    result.z = z;
    return result;
}
