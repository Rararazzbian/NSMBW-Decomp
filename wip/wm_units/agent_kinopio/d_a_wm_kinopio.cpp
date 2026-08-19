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
#include <game/framework/f_manager.hpp>
#include <game/framework/f_base.hpp>
#include <game/bases/d_a_wm_player.hpp>
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

// @unofficial dWmMapModel_c::GetEndNodePos(mVec3_c&) -- not in the real
// header (dWmMapModel_c is currently opaque padding only). Mangled name
// confirmed directly from the target
// (GetEndNodePos__13dWmMapModel_cFR7mVec3_c). daWmMap_c's own mModels[4]
// array and currIdx field are ALREADY correctly declared in the real
// header (game/bases/d_a_wm_map.hpp) -- independently confirmed here:
// offsetof(daWmMap_c, mModels) == 0x1a0 and the observed field read at
// map+0x338c matches offsetof(currIdx) == 0x1a0 + 4*sizeof(dWmMapModel_c)
// == 0x1a0 + 4*0xbf8 == 0x338c exactly, so `map->mModels[map->currIdx]`
// needs no raw-offset arithmetic at all -- only this one method is
// missing.
extern "C" void GetEndNodePos__13dWmMapModel_cFR7mVec3_c(dWmMapModel_c *self, mVec3_c &out);

// @unofficial stepCutscene70() externs -- all read directly off the
// target's own call sites (mangled/raw names), not guessed.
extern "C" void fn_2_192920(dWmActor_c *);
extern "C" int fn_2_192930(dWmActor_c *);
// @unofficial fn_80103520 -- distinct from the already-landed fn_80103420
// (d_a_wm_kinoko_base.cpp): this one's result IS used (stored into
// m_1b0, the effect-ID field), so it returns int, not void. Same argument
// shape otherwise (mgr, effect id, model, kind name, 0, 0).
extern "C" int fn_80103520(dWmEffectManager_c *mgr, int effectId, m3d::mdl_c *model,
                            const char *kind, int, int);
extern "C" void fn_80105170(dWmSeManager_c *mgr, int a, int b, u8 c, float d);
namespace dWmLib {
    void InitKinopioCourse();
}

// @unofficial fn_80100640 -- cross-module DOL call, unnamed in both symbol
// tables. Same signature already established and used with this exact
// argument shape (daWmMap_c*, node name, 0) in wip/wm_units/agent_anchor's
// d_a_wm_anchor.cpp and multiple other WM units; returns a node/model
// pointer, null on lookup failure.
extern "C" void *fn_80100640(daWmMap_c *map, const char *name, int unused);

// @unofficial stepCutscene70() case 14 externs -- read directly off the
// target's own call sites, not guessed.
// fn_2_16AE70 == daWmKinoballoon_c::triggerFirstStartMove() (confirmed:
// identical address 0x0016AE70 in wip/wm_units/agent_kinoballoon's own
// draft, a static method of the sibling class living in this same REL) --
// a genuine cross-class call, no arguments, no explicit `this` setup in
// the target at either of the two paths reaching this call.
extern "C" void fn_2_16AE70();
// fn_2_1998E0 -- not identified by name; called with
// daWmPlayer_c::ms_instance as its sole argument (no vtable indirection,
// a direct bl), same raw-extern technique as fn_2_192920/fn_2_192930
// above.
extern "C" void fn_2_1998E0(daWmPlayer_c *player);

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
    // @unofficial ROUND 13 CORRECTION: this used to be a hand-authored
    // duplicate `static dWmLib::ForceInCourseList_t sForceList = {...}`
    // with the SAME literal values as dWmLib::sc_ForceList's own castle
    // entry (d_wm_lib.hpp). That was never a coincidence-worth-a-comment,
    // it was the ANSWER: re-reading fn_2_16D1E0's target disassembly
    // fresh (not trusting the inherited "kinopio has its own separate
    // sc_ForceList-shaped entry" framing) shows the function constructs
    // EXACTLY ONE ForceInCourseList_t object, at address lbl_2_data_45C90
    // -- the same address, same fields, same values previously attributed
    // to "our own" sForceList. There was never a second object. This
    // TU's own code never referenced `sForceList` anywhere outside its
    // own declaration (confirmed by grep), so it was a pure, unused
    // duplicate of the header's own static, and duplicating it is exactly
    // what produced the extra .ctors entry every round-3/4 variant kept
    // measuring. Deleted. `dWmLib::sc_ForceList` (pulled in by the
    // already-required `#include <game/bases/d_wm_lib.hpp>`, needed
    // regardless for `ForceInCourseList_t`/`IsSingleEntry()`/
    // `InitKinopioCourse()`) is the ONE object the target actually
    // constructs, and this TU is simply the (or a) translation unit where
    // that internal-linkage header static happens to be emitted.

    // @unofficial lbl_2_data_45CBC, a POINTER variable (not an array) --
    // see the long comment near stepCutscene70() for the full
    // `.rela.data`-relocation-based derivation. Shared by
    // stepCutscene70() cases 12 and 14 (3 call sites total, all reading
    // this same pointer's value).
    static const char *sW101 = "W101";

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

// @unofficial lbl_2_data_45D00 -- SECTION-PLACEMENT bug found and fixed
// this round: a NAMED `static const char[]` (the form used for this
// string until now) compiles to .rodata under this project's MWCC
// settings, but the target's own `lbl_2_data_45D00` label proves the
// string lives in .data. processCutsceneCommand's existing inline
// "W103"/"W102" literals (used directly as call arguments, never named)
// already land in .data correctly -- confirmed directly against both the
// target's own .data dump (dump_data.py) and this unit's own compiled
// draft.o (dump_obj_section.py), which showed the old named
// "kinopio_all_root" landing in .rodata alongside the float table,
// wrongly reachable through the same base register (r31) already live
// for that table. Fixed by using a bare inline literal at both call
// sites below instead of a named constant (matching the
// already-.data-correct W103/W102 idiom). Named `static const float`s
// (k8B50_shared, sCamParams below) are NOT affected -- floats already
// correctly land in .rodata either way, matching the target.
//
// @unofficial lbl_2_data_45CBC (sW101) is declared up near sForceList,
// not here, so its .data POOL POSITION matches the target -- see that
// declaration's own comment for the full relocation-based derivation
// (it is a pointer VARIABLE, not an array/literal; caught by a direct
// .rela.data relocation lookup against
// bin/dtkspl/d_basesNP/obj/auto_04_00044A68_data.o).
namespace {
    // @unofficial lbl_2_rodata_8B20 -- a 4-float table pointer stored into
    // dWCamera_c's own +0x71c field in case 12. First 3 floats (0.1f,
    // 12.0f, 1.0f) match wip/wm_units/agent_start's own sc_CamParams table
    // exactly; the 4th differs per call site (agent_start uses 0.0f, this
    // unit's case 12 uses 100.0f) -- same "camera ease-curve parameter
    // block" family, case-specific last entry.
    static const float sCamParams[4] = {0.1f, 12.0f, 1.0f, 100.0f};
}

void daWmKinopio_c::stepCutscene70() {
    calcSpeed();
    posMove();

    if (mpMdlMng->mpMdl->isFootStepTiming()) {
        fn_80105170(dWmSeManager_c::m_pInstance, 0x10, 2, mpMdlMng->mpMdl->m_152, mSpeedF);
    }

    switch (m_1a8) {
    case 0:
        if (mPos.x > m_19c.x - 100.0f) {
            // @unofficial re-read fresh this round: the multiply was
            // WRONG (previously `* 1.0f`, a no-op I must have misread or
            // placeholder'd) -- the target actually does
            // `fmuls f2, C[0x58], dist` where C[0x58] (rodata @ r31+0x58)
            // is -2.0f, confirmed against a freshly re-derived, complete
            // rodata table (0x8b10-0x8bac) rather than the partial one
            // used originally. This -2.0f was previously MISSING from
            // this draft entirely, and is at least part of why this
            // unit's compiled .rodata (0x84 bytes) was measured SHORT
            // against the target's 0x90 -- a real, evidenced case of "the
            // pool is short because the content referencing it was never
            // written," not a declaration-order problem for this specific
            // constant.
            //
            // @unofficial STILL OPEN, not guessed: the divisor is
            // genuinely `15` (confirmed by value), but the target reaches
            // it via a full runtime int-to-double bit-trick conversion
            // (li r3,0xf; lis r0,0x4330; xoris; stw/stw; lfd; fsubs),
            // TWICE, rather than a pooled float immediate. Every literal
            // form tried here (`15.0f`, `(int)15`) gets constant-folded
            // by this same compiler into a plain `lfs`, which does NOT
            // reproduce the target's shape. The true source expression
            // for this divisor is therefore something this compiler
            // cannot see as compile-time-constant -- an unidentified
            // field, parameter, or macro that evaluates to 15 at this
            // call site, not a plain numeric literal. Left as a literal
            // rather than inventing a fake runtime source for it.
            float dist = (m_19c.x - mPos.x) * -2.0f;
            float speed = dist / 15.0f;
            m_194 = speed / 15.0f;
            mpMdlMng->mpMdl->setAnm(2, 3.0f, 5.0f, 0.0f);
            m3d::mdl_c *mdl = mpMdlMng->mpMdl->getBodyMdl();
            m_1b0 = fn_80103520(dWmEffectManager_c::m_pInstance, 2, mdl, "kinopio_all_root", 0, 0);
            m_1a8 = 1;
        }
        break;
    case 1:
        mSpeedF = mSpeedF + m_194;
        if (m_198 > 0) {
            m_198 = m_198 - 1;
        } else {
            mSpeedF = 0.0f;
            mpMdlMng->mpMdl->setAnm(0xab, 1.0f, 5.0f, 0.0f);
            m_198 = 0xa;
            m_1a8 = 2;
            m_1ac = 0;
            dWmEffectManager_c::m_pInstance->endEffect(m_1b0);
            dWmSeManager_c::m_pInstance->playSound(0x35, mPos, 1);
        }
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
        if (m_198 > 0) {
            m_198 = m_198 - 1;
        } else if (!m_1b8) {
            m_1a8 = 0x11;
        } else {
            setDirection(mVec3_c(-1.0f, 0.0f, 0.0f));
            mSpeedF = 6.0f;
            mpMdlMng->mpMdl->setAnm(2, 3.0f, 5.0f, 0.0f);
            m3d::mdl_c *mdl = mpMdlMng->mpMdl->getBodyMdl();
            m_1b0 = fn_80103520(dWmEffectManager_c::m_pInstance, 2, mdl, "kinopio_all_root", 0, 0);
            m_1a8 = 0xb;
        }
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
    case 12: {
        // @unofficial dWCamera_c's real layout still isn't in the shared
        // header (only `char pad[0x4f8]` before mViewClip) -- but this is
        // NOT a header-fix situation like getBodyMdl: the accepted, LANDED
        // technique for these exact same six fields
        // (source/d_basesNP/bases/d_a_wm_note.cpp's own processCutsceneCommand,
        // case 0x20) is a local raw u8* cast confined to this .cpp, not a
        // header change. wip/wm_units/agent_start independently hit the
        // identical six offsets with the same apparent types. Reusing that
        // established, already-landed technique here.
        daWmMap_c *map = daWmMap_c::m_instance;
        dWCamera_c *camera = dWCamera_c::m_instance;
        mVec3_c endPos;
        GetEndNodePos__13dWmMapModel_cFR7mVec3_c(&map->mModels[map->currIdx], endPos);
        m_19c = GetPos__9daWmMap_cFPCc(daWmMap_c::m_instance, sW101);
        m_19c.y = endPos.y;
        m_19c.z = endPos.z;
        u8 *cam = (u8 *) camera;
        *(u32 *) (cam + 0x604) = 2;
        *(mVec3_c **) (cam + 0x5f4) = &m_19c;
        *(u32 *) (cam + 0x5f0) = 0;
        *(bool *) (cam + 0x624) = false;
        *(u32 *) (cam + 0x608) = 0;
        *(const float **) (cam + 0x71c) = sCamParams;
        m_198 = 0x3c;
        m_1a8 = 0xd;
        break;
    }
    case 13:
        if (*(int *) ((u8 *) dWCamera_c::m_instance + 0x5f4) == 0) {
            if (m_198 > 0) {
                m_198 = m_198 - 1;
            } else {
                m_1a8 = 0xe;
            }
        }
        break;
    case 14: {
        dWmLib::InitKinopioCourse();
        // @unofficial fManager_c::searchBaseByProfName(fProfile::WM_KINOBALLOON,
        // nullptr) -- profile id read directly off the target (0x2a7 ==
        // 679). NOT fProfile::WM_BUBBLE (0x2a6, confirmed by compiling
        // against the real enum and comparing the resulting `li r3,...`
        // immediate against the target's 0x2a7) -- ties together with the
        // fn_2_16AE70 call below (daWmKinoballoon_c::triggerFirstStartMove()):
        // this finds a kinoballoon actor and arms it as part of starting
        // this cutscene course. Mangled name matches fManager_c's own
        // static (not dBase_c's convenience wrapper), same call shape
        // already used elsewhere in this project (d_a_en_door.cpp etc).
        fBase_c *balloon = fManager_c::searchBaseByProfName(fProfile::WM_KINOBALLOON, nullptr);
        if (balloon) {
            // @unofficial fn_80100640's returned node's field at +0xc (real
            // type unidentified) is stored at balloon+0x184 (the
            // kinoballoon's own real type, daWmKinoballoon_c, is declared
            // in wip/wm_units/agent_kinoballoon but not in include/, so
            // accessed via raw offset cast here, same idiom as
            // lbl_2_bss_11B70 elsewhere in this function).
            void *node = fn_80100640(daWmMap_c::m_instance, sW101, 0);
            *(u32 *) ((u8 *) balloon + 0x184) = node ? *(u32 *) ((u8 *) node + 0xc) : 0;
            mVec3_c pos = GetPos__9daWmMap_cFPCc(daWmMap_c::m_instance, sW101);
            *(mVec3_c *) ((u8 *) balloon + 0xac) = pos;
        }
        fn_2_16AE70();
        fn_2_1998E0(daWmPlayer_c::ms_instance);
        m_198 = 0x3c;
        m_1a8 = 0xf;
        break;
    }
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
