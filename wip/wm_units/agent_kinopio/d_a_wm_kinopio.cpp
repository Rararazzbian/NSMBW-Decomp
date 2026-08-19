#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_a_wm_kinopio.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_wm_csv_data.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_w_camera.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/bases/d_wm_effect_manager.hpp>
#include <constants/game_constants.h>

// @unofficial lbl_2_bss_11B70 -- a shared .bss singleton pointer, real
// type not yet identified (accessed via raw offsets in stepCutscene70()).
extern void *lbl_2_bss_11B70;

// @unofficial cross-module DOL call, unnamed in both symbol tables.
extern "C" int fn_800FCB30(int);

// @unofficial daWmMap_c::GetPos(const char *) -- not in the real header
// (only GetPos(int) is declared there); mangled name confirmed directly
// from the target (GetPos__9daWmMap_cFPCc). Declared as the equivalent
// free function taking an explicit `this`, same technique as the
// project's other raw cross-TU externs.
extern "C" mVec3_c GetPos__9daWmMap_cFPCc(daWmMap_c *self, const char *nodeName);

// @unofficial stepCutscene70() externs -- all read directly off the
// target's own call sites (mangled/raw names), not guessed.
extern "C" void fn_2_192920(dWmActor_c *);
extern "C" int fn_2_192930(dWmActor_c *);
extern "C" void fn_80105170(dWmSeManager_c *mgr, int a, int b, u8 c, float d);
namespace dWmLib {
    void InitKinopioCourse();
}

// @unofficial KNOWN ISSUE, not resolved: the target's .ctors section has
// exactly 1 entry for this unit; the real source therefore does NOT
// include d_wm_lib.hpp (that header's own `static ForceInCourseList_t
// sc_ForceList[] = {...}` carries a side-effecting dynamic initializer --
// it initialises its mVec3_c field via a CONSTRUCTOR CALL, not brace
// aggregate-init, so including it would add a second real .ctors entry).
// Retried declaring only what's used (`namespace dWmLib { bool
// IsSingleEntry(); struct ForceInCourseList_t {...}; }`, no include) with
// the vector field ALSO constructor-called (`mVec3_c(2160.0f, -30.0f,
// -478.0f)`, matching sc_ForceList's own shape exactly, not brace-init) --
// still no `__register_global_object`/`fn_2_16D270` generated; identical
// result to every earlier attempt. So the brace-vs-constructor distinction
// was not the discriminator for this specific include-vs-no-include gap
// (both this unit's variants already used a constructor call throughout).
// Whatever the real trigger is remains unidentified. Parked per
// instruction: including the header yields one MATCH
// (fn_2_16D270) and costs one spurious extra .ctors entry on fn_2_16D1E0,
// the better of the two imperfect states measured.

ACTOR_PROFILE(WM_KINOPIO, daWmKinopio_c, 0);

namespace {
    // @unofficial file-local dWmLib::ForceInCourseList_t entry, registered
    // dynamically because mLevel is patched at load from the runtime
    // global dCsvData_c::c_CASTLE_ID (not a compile-time constant, which is
    // why this needs a guarded/dynamic .ctors init instead of pure .data).
    // Same world/node-name/entrance/position as dWmLib::sc_ForceList's own
    // castle entry (d_wm_lib.hpp) -- kinopio evidently sits at the same
    // course-entry point.
    static dWmLib::ForceInCourseList_t sForceList = {
        WORLD_7, "F7C0", WORLD_7, dCsvData_c::c_CASTLE_ID, 4, "W7C0",
        mVec3_c(2160.0f, -30.0f, -478.0f)
    };

    // @unofficial fn_2_16C5D0 (unusedStub) is the sole PTMF-table target --
    // confirmed by counting relocations in .rodata 0x8b10-0x8bb0 (only one,
    // at 0x8b3c, resolving to fn_2_16C5D0) rather than assuming an entry
    // count from dtk's own (unreliable) object-size report.
    typedef void (daWmKinopio_c::*StepFunc_t)();
    static const StepFunc_t sStepTable[1] = {
        &daWmKinopio_c::unusedStub,
    };

    // @unofficial lbl_2_rodata_8B10+0x38/0x3c/0x40/0x44, declared in the
    // target's own memory order so the isolated compile's constant pool
    // lands the same way (matches the "consolidate constants" lesson --
    // scattered anonymous literals pool in source-usage order, not target
    // memory order).
    // @unofficial lbl_2_rodata_8B10+0x40 -- the one constant shared
    // between calcModel() and resetPosition(); named so the compiler
    // pools it once instead of twice. The other three (+0x38/0x3c/0x44)
    // are each used by only one function and are left as plain literals.
    static const float k8B50_shared = 0.5f;
}

daWmKinopio_c::daWmKinopio_c() {}

daWmKinopio_c::~daWmKinopio_c() {
    if (mpMdlMng) {
        delete mpMdlMng;
    }
}

int daWmKinopio_c::create() {
    createModel();
    mClipSphere.set(mPos, 100.0f);
    calcModel();
    resetPosition();
    mScale.x = 1.899999976158142f;
    mScale.y = 1.899999976158142f;
    mScale.z = 1.899999976158142f;
    return SUCCEEDED;
}

int daWmKinopio_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    if (csSeqMng->FUN_80915600()) {
        processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    } else {
        (this->*sStepTable[m_190])();
    }
    checkAnmLoop();
    calcModel();
    return SUCCEEDED;
}

int daWmKinopio_c::draw() {
    mpMdlMng->draw();
    DrawShadow(true);
    return SUCCEEDED;
}

int daWmKinopio_c::doDelete() {
    return 1;
}

void daWmKinopio_c::createModel() {
    mpMdlMng = new dPyMdlMng_c((dPyMdlMng_c::ModelType_e) 4);
    mpMdlMng->create(1, 1, (dPyMdlMng_c::SceneType_e) 1);

    static const char sArcName[] = "character_SV";
    static const char sPath[] = "g3d/model.brres";
    CreateShadowModel(sArcName, sPath, sArcName, true);

    if (checkSpawnGate()) {
        m_1b8 = dWmActor_c::construct((ProfileName) 0x28f, this, 0x10000, nullptr, nullptr);
    }
}

void daWmKinopio_c::calcModel() {
    CalcShadow(k8B50_shared, 0.800000011920929f);
    mpMdlMng->calc(mPos, mAngle, mScale);
    mpMdlMng->play();
}

void daWmKinopio_c::resetPosition() {
    mPos = mVec3_c::Zero;
    mPos.x = -500.0f;
    mpMdlMng->mpMdl->setAnm(4, 5.0f, 5.0f, k8B50_shared);
    m_1b4 = 0;
    resetStep();
}

void daWmKinopio_c::resetStep() {
    m_190 = 0;
}

void daWmKinopio_c::unusedStub() {
}

void daWmKinopio_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) {
        return;
    }

    if (isFirstFrame && cutsceneCommandId == 0x70) {
        mSpeedF = 0.800000011920929f;
        setDirection(mVec3_c(-500.0f, 0.5f, 0.5f));

        if (!checkSpawnGate()) {
            mVec3_c p1 = GetPos__9daWmMap_cFPCc(daWmMap_c::m_instance, "W103");
            mVec3_c p2 = GetPos__9daWmMap_cFPCc(daWmMap_c::m_instance, "W102");
            m_19c = p1 * 0.0f + p2 * 5.0f;
        } else {
            mVec3_c p1 = GetPos__9daWmMap_cFPCc(daWmMap_c::m_instance, "W103");
            mVec3_c p2 = GetPos__9daWmMap_cFPCc(daWmMap_c::m_instance, "W102");
            m_19c = p1 * 0.0f + p2 * 8.0f;
        }

        m_1a8 = 0;
        m_198 = 0xf;
    }

    if (cutsceneCommandId == 0x70) {
        stepCutscene70();
    } else {
        mIsCutEnd = true;
    }
}

void daWmKinopio_c::stepCutscene70() {
    calcSpeed();
    posMove();

    if (mpMdlMng->mpMdl->isFootStepTiming()) {
        fn_80105170(dWmSeManager_c::m_pInstance, 0x10, 2, mpMdlMng->mpMdl->m_152, mSpeedF);
    }

    switch (m_1a8) {
    case 0:
        // @unofficial NOT fully decoded -- placeholder.
        break;
    case 1:
        // @unofficial NOT fully decoded -- placeholder.
        break;
    case 2:
        if (m_1ac > 1) {
            m_198 = 0xb4;
            // @unofficial lbl_2_bss_11B70 -- a shared, not-yet-typed
            // singleton pointer read from .bss; fields at +0x544/+0x546/
            // +0x55c accessed via raw offset since its real type isn't
            // identified. NOT fully decoded otherwise.
            u8 *mgr = *(u8 **) &lbl_2_bss_11B70;
            *(bool *) (mgr + 0x544) = true;
            *(bool *) (mgr + 0x546) = true;
            *(u32 *) (mgr + 0x55c) = 0xd;
            m_1a8 = 4;
        }
        break;
    case 3:
        break;
    case 4:
        if (m_198 > 0) {
            m_198 = m_198 - 1;
        } else {
            m_198 = 0x78;
            m_1a8 = 5;
        }
        break;
    case 5:
        if (m_198 > 0) {
            m_198 = m_198 - 1;
        }
        if (m_198 == 0) {
            if (dGameKey_c::m_instance->mRemocon[mPad::g_currentCoreID]->mDownButtons & 0x900) {
                u8 *mgr = *(u8 **) &lbl_2_bss_11B70;
                *(bool *) (mgr + 0x545) = true;
                if (dWmLib::IsSingleEntry()) {
                    u8 *obj = *(u8 **) (mgr + 0x538);
                    if (obj) {
                        *(bool *) (obj + 0x251) = true;
                        *(u32 *) (obj + 0x254) = 1;
                    }
                } else {
                    u8 *obj = *(u8 **) (mgr + 0x538);
                    if (obj) {
                        *(bool *) (obj + 0x251) = true;
                        *(u32 *) (obj + 0x254) = 1;
                    }
                }
                *(bool *) (mgr + 0x54d) = true;
                m_1a8 = 6;
            }
        }
        break;
    case 6:
        // @unofficial lbl_2_bss_11B70's real type not identified (see
        // MAPPING.md) -- ownership check shows it referenced from nearly
        // every corner of this module, ruled out dCsSeqMng_c/
        // dWmEffectManager_c/dGameKey_c by field-range mismatch. Kept as
        // raw offset casts per the established two-landed-units handling.
        if (*(u8 *) (*(u8 **) &lbl_2_bss_11B70 + 0x54d) == 0) {
            m_198 = 0xb4;
            mpMdlMng->mpMdl->setAnm(0, 1.0f, 20.0f, 0.0f);
            u8 *mgr = *(u8 **) &lbl_2_bss_11B70;
            *(bool *) (mgr + 0x544) = true;
            *(bool *) (mgr + 0x546) = true;
            *(u32 *) (mgr + 0x55c) = 0xe;
            m_1a8 = 7;
        }
        break;
    case 7:
        if (m_198 > 0) {
            m_198 = m_198 - 1;
        } else {
            m_198 = 0x78;
            m_1a8 = 8;
            m_1ac = 0;
        }
        break;
    case 8:
        if (m_198 > 0) {
            m_198 = m_198 - 1;
        }
        if (m_198 == 0) {
            if (dGameKey_c::m_instance->mRemocon[mPad::g_currentCoreID]->mDownButtons & 0x900) {
                *(bool *) (*(u8 **) &lbl_2_bss_11B70 + 0x545) = true;
                m_1a8 = 9;
            }
        }
        break;
    case 9:
        m_198 = 0x1e;
        m_1a8 = 0xa;
        break;
    case 10:
        // @unofficial NOT fully decoded -- placeholder.
        break;
    case 11:
        if (mPos.x < -500.0f) {
            clearSpeedAll();
            dWmEffectManager_c::m_pInstance->endEffect(m_1b0);
            mpMdlMng->mpMdl->setAnm(0, 1.0f, 0.0f, 0.0f);
            if (checkSpawnGate()) {
                m_1a8 = 0xe;
            } else {
                m_1a8 = 0x13;
            }
        }
        break;
    case 12:
        // @unofficial NOT fully decoded -- placeholder.
        break;
    case 13:
        if (*(int *) ((u8 *) dWCamera_c::m_instance + 0x5f4) == 0) {
            if (m_198 > 0) {
                m_198 = m_198 - 1;
            } else {
                m_1a8 = 0xe;
            }
        }
        break;
    case 14:
        // @unofficial NOT fully decoded (starts with dWmLib::InitKinopioCourse()) -- placeholder.
        dWmLib::InitKinopioCourse();
        break;
    case 15:
        if (m_198 > 0) {
            m_198 = m_198 - 1;
        } else {
            m_1a8 = 0x13;
        }
        break;
    case 16:
        if (*(int *) ((u8 *) dWCamera_c::m_instance + 0x5f4) == 0) {
            m_1a8 = 0x13;
        }
        break;
    case 17:
        fn_2_192920(m_1b8);
        m_1a8 = 0x12;
        break;
    case 18:
        if (!m_1b4) {
            if (m_1b8->mPos.x <= mPos.x) {
                dWmSeManager_c::m_pInstance->playSound(0x67, mPos, 1);
                m_1b4 = 1;
            }
        }
        if (m_1b4) {
            mpMdlMng->mpMdl->setAnm(0x54, 5.0f, 5.0f, 0.0f);
            mPos.x = m_1b8->mPos.x - 20.0f;
        }
        if (fn_2_192930(m_1b8)) {
            mVisible = false;
            m_1a8 = 0xe;
        }
        break;
    case 19:
        setCutEnd();
        break;
    }
}

void daWmKinopio_c::checkAnmLoop() {
    if ((u32) (m_1a8 - 2) <= 7) {
        if (mpMdlMng->getLastFrame() == mpMdlMng->mpMdl->mAnm.getFrame()) {
            mpMdlMng->mpMdl->setFrame(0.0f);
            m_1ac = m_1ac + 1;
        }
    }
    if (m_1ac > 1000) {
        m_1ac = 0;
    }
}

void daWmKinopio_c::startJump(const char *nodeName, const JumpParam_t *param) {
    mVec3_c pos;
    daWmMap_c::m_instance->GetNodePos(nodeName, pos);
    _initDemoJumpBase(pos, 0, param->mFrames, param->mSpeed,
                       1.899999976158142f * param->mStartScaleSrc,
                       1.899999976158142f * param->mTargetScaleSrc,
                       mVec3_c::Ex);
    m_1b4 = 0;
}

bool daWmKinopio_c::checkSpawnGate() {
    bool result = false;
    if (dWmLib::IsSingleEntry()) {
        if (!fn_800FCB30(0)) {
            result = true;
        }
    }
    return result;
}
