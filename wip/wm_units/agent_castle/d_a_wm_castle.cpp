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
// A non-const, namespace/file-scope array with an mVec3_c member (the same shape as
// dWmLib::sc_ForceList) reproduces .data and .rodata byte-for-byte and brings __sinit from 52 to
// 16 differing instructions; four further shapes (function-local static, single struct instead
// of array, inline-accessor-in-initialiser) were each tried and measurably worse. See this
// task's reports for the full five-shape comparison. The guard's exact origin inside __sinit
// remains unexplained; .bss is 4 bytes over (two __register_global_object blocks vs the
// target's apparent one).
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
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1, nullptr);

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

    if (!dWmLib::isSpecialWorld() && dWmLib::isKoopaShipOnCurrentWorld()) {
        if (mCutscene == dCsSeqMng_c::SMC_DEMO_W1_CASTLE_CLR || mCutscene == dCsSeqMng_c::SMC_DEMO_W3_CASTLE_CLR) {
            mVec3_c pos; // see function comment -- uninitialized in the target
            construct(fProfile::WM_KOOPASHIP, this, 2, &pos, nullptr);

            if (mCutscene == dCsSeqMng_c::SMC_DEMO_W3_CASTLE_CLR) {
                mVec3_c pos2(mPos.x, mPos.y, mPos.z - 100.0f);
                construct(fProfile::WM_KOOPAJR, this, 0, &pos2, nullptr);
            }
        }
    }

    if (!IsCourseClear()) {
        mVec3_c pos = mVec3_c::Zero;
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
