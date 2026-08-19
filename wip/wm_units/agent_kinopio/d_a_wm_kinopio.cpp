#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_a_wm_kinopio.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_wm_csv_data.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <constants/game_constants.h>

// @unofficial cross-module DOL call, unnamed in both symbol tables.
extern "C" int fn_800FCB30(int);

// @unofficial KNOWN ISSUE, not resolved: the target's .ctors section has
// exactly 1 entry for this unit; the real source therefore does NOT
// include d_wm_lib.hpp (that header's own `static ForceInCourseList_t
// sc_ForceList[] = {...}` carries a side-effecting dynamic initializer,
// so including it would add a second .ctors entry). Tried declaring only
// what's used instead of including the header -- `namespace dWmLib { bool
// IsSingleEntry(); struct ForceInCourseList_t { ...same 7 fields... }; }`,
// no include -- three variants (plain, with an explicit empty `~ForceIn
// CourseList_t(){}`, struct at namespace scope vs nested): ALL of them
// make MWCC stop emitting `__register_global_object` for `sForceList`
// entirely -- it just does a bare unguarded store, no atexit/array-dtor
// registration at all, unlike the target (and unlike this same object
// when d_wm_lib.hpp IS included). The trigger for that codegen difference
// was not identified this round -- it is not simply "does the compiler
// see a non-trivial destructor" (mVec3_c's IS user-declared, present
// either way). Reverted to the real include, which at least reproduces
// fn_2_16D270 (the generated __arraydtor callback) exactly; fn_2_16D1E0
// itself still carries the extra sc_ForceList work on top of its own.

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
    static const float k8B48 = -500.0f;
    static const float k8B4C = 5.0f;
    static const float k8B50_2 = 0.5f;
    static const float k8B44 = 0.800000011920929f;
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
    CalcShadow(k8B50_2, k8B44);
    mpMdlMng->calc(mPos, mAngle, mScale);
    mpMdlMng->play();
}

void daWmKinopio_c::resetPosition() {
    mPos = mVec3_c::Zero;
    mPos.x = k8B48;
    mpMdlMng->mpMdl->setAnm(4, k8B4C, k8B4C, k8B50_2);
    m_1b4 = 0;
    resetStep();
}

void daWmKinopio_c::resetStep() {
    m_190 = 0;
}

void daWmKinopio_c::unusedStub() {
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

bool daWmKinopio_c::checkSpawnGate() {
    bool result = false;
    if (dWmLib::IsSingleEntry()) {
        if (!fn_800FCB30(0)) {
            result = true;
        }
    }
    return result;
}
