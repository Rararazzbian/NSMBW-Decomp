#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_course.hpp>
#include <game/bases/d_wm_lib_course_ext.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_wm_effect_manager.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_info.hpp>
#include <game/framework/f_manager.hpp>

ACTOR_PROFILE(WM_COURSE, daWmCourse_c, 0);

// @unofficial Unnamed in the symbol map (fn_80103420, DOL 0x80103420, 0x74 B).
// Called as dWmEffectManager_c::m_pInstance->fn_80103420-shape(kind, model,
// name, 0, 0) from updateOpenAnim() case 1 -- the effect-manager "this" is
// loaded explicitly into r3 at the call site rather than via an implicit
// member-call sequence, so it is declared here as a plain extern "C" function
// taking the manager pointer as its first argument, per the syms.txt
// fn_800XXXXX convention (see source/dol/bases/d_a_player_demo_manager.cpp).
extern "C" void fn_80103420(dWmEffectManager_c *mgr, int kind, m3d::bmdl_c &model, const char *name, int, int);

// @unofficial Unnamed in the symbol map (fn_2_191BF0, .text:0x191BF0, 0x3C B),
// owned by a TU inside d_basesNP that is not yet landed. syms.txt only carries
// DOL (0x8xxxxxxx) addresses -- there is no mechanism there for a REL-internal
// symbol, so this cannot resolve at link time until that TU lands (same class
// of blocker as d_a_wm_kinoko_1up.cpp). At both call sites below, r3 holds a
// leftover address from an unrelated preceding load (not `this`, not a search
// result) right before the call, so the real signature is very likely
// argument-less; declared that way, but unverifiable while blocked.
extern "C" bool fn_2_191BF0();

// @unofficial Unnamed in the symbol map (fn_2_189A20, .text:0x189A20, 0x64 B;
// fn_2_189990, .text:0x189990, 0x88 B), both owned by a different not-yet-
// landed TU inside d_basesNP -- fn_2_189990 shows up as the destination of a
// switch jump table in two other wip units' `.data` (`agent_koopa_castle` and
// `agent_sandpillar`'s `fn_2_189990+0x20 .. +0x74` entries), which is the
// function's OWN internal dispatch, not a class vtable; it tells us nothing
// about this call site's argument types. Same class of blocker as
// fn_2_191BF0/fn_80103420: syms.txt only carries DOL addresses, so a
// REL-internal symbol from an unlanded TU cannot resolve here. At the one
// call site below, `fn_2_189A20(world)` is called with no `this` setup and
// its return sits untouched in r3 right up to the `bl fn_2_189990`, which is
// only possible if that return value IS fn_2_189990's first argument -- the
// two calls chain. Declared as a plain int-in/pointer-out pair on that
// evidence; unverifiable further while blocked.
extern "C" void *fn_2_189A20(int world);
extern "C" void fn_2_189990(void *obj, int, int);

static const char *sResAnmNames[daWmCourse_c::ANIM_COUNT] = {
    "cobCourseClear",
    "cobCourseHelp",
    "cobCourseOpen"
};

// Read directly off the target's rodata pool (lbl_2_rodata_87C0, words
// +0x4/+0x8/+0xc: 0x1,0x0,0x0), which createModel's per-index setPlayMode
// call indexes with a pointer that increments by 4 each loop iteration --
// an array read, not a repeated constant. m3d::FORWARD_ONCE == 1,
// m3d::FORWARD_LOOP == 0 (m_3d/banm.hpp), matching the byte values exactly.
static const m3d::playMode_e sPlayModes[daWmCourse_c::ANIM_COUNT] = {
    m3d::FORWARD_ONCE,
    m3d::FORWARD_LOOP,
    m3d::FORWARD_LOOP
};

daWmCourse_c::daWmCourse_c() : mOpenState(0) {}
daWmCourse_c::~daWmCourse_c() {}

// Matches fn_2_160610's shape. GetCourseTypeFromCourseNo()==8 is the "Koopa
// ship" course type; the branches below construct either a WM_KOOPASHIP or a
// WM_ANCHOR child actor at a stack-local copy of mVec3_c::Zero (staged
// through a local, not `&mVec3_c::Zero` directly -- see AGENT_CONTEXT.md
// lever 6) and fire one of several airship-themed dCsSeqMng_c cutscenes.
// IsCourseClear()/IsCourseFirstOmoteClear() are each called twice at
// different points in the target's bytes rather than cached, which is why
// the same calls appear twice below instead of once in a shared local.
int daWmCourse_c::create() {
    createModel();
    calcModel();

    mUnk250 = false;
    mClipSphere.set(mPos, 80.0f);

    if (!dWmLib::isSpecialWorld() && dWmLib::GetCourseTypeFromCourseNo((int)ACTOR_PARAM(CourseNo)) == 8) {
        u8 world = dScWMap_c::m_WorldNo;

        if (dWmLib::isKoopaShipOnCurrentWorld()) {
            if (!IsCourseClear() || IsCourseFirstOmoteClear()) {
                int param = 3;
                if (IsCourseFirstOmoteClear() && dScWMap_c::m_WorldNo != 7) {
                    param = 4;
                }

                mVec3_c zeroPos = mVec3_c::Zero;
                dWmActor_c *ship = dWmActor_c::construct(fProfile::WM_KOOPASHIP, this, param, &zeroPos, nullptr);

                if (world != 7) {
                    if (IsCourseFirstClear()) {
                        daWmPlayer_c::ms_instance->mPos = ship->mPos;
                        dCsSeqMng_c::ms_instance->FUN_801017c0(dCsSeqMng_c::SMC_DEMO_AIRSHIP_GONEXT, nullptr, nullptr, 200);
                    }
                } else {
                    if (IsCourseFirstOmoteClear()) {
                        fn_2_189990(fn_2_189A20(world), 0, 4);
                        daWmPlayer_c::ms_instance->mPos = ship->mPos;
                        dCsSeqMng_c::ms_instance->FUN_801017c0(dCsSeqMng_c::SMC_DEMO_KOOPACASTLEAPPEAR, nullptr, nullptr, 200);
                    }
                }
            } else if (IsCourseClear()) {
                if (world == 7) {
                    mVec3_c zeroPos = mVec3_c::Zero;
                    dWmActor_c::construct(fProfile::WM_KOOPASHIP, this, 3, &zeroPos, nullptr);

                    if ((int)ACTOR_PARAM(CourseNo) == 0x25 && dInfo_c::m_instance->m_54 == 0x25
                        && (u32)(dInfo_c::m_instance->m_60 - 2) <= 3) {
                        dCsSeqMng_c::ms_instance->FUN_801017c0(dCsSeqMng_c::SMC_DEMO_AIRSHIP_CLEAR, nullptr, nullptr, 200);
                    }
                } else {
                    mVec3_c zeroPos = mVec3_c::Zero;
                    dWmActor_c::construct(fProfile::WM_ANCHOR, this, 0, &zeroPos, nullptr);
                }
            }

            if (IsCourseFailed() && (!IsCourseClear() || world == 7)) {
                dCsSeqMng_c::ms_instance->FUN_801017c0(dCsSeqMng_c::SMC_DEMO_AIRSHIP_COURSE_OUT, nullptr, nullptr, 200);
            }
        } else {
            mVec3_c zeroPos = mVec3_c::Zero;
            dWmActor_c::construct(fProfile::WM_ANCHOR, this, 0, &zeroPos, nullptr);

            if (IsCourseFirstClear()) {
                dCsSeqMng_c::ms_instance->FUN_801017c0(dCsSeqMng_c::SMC_DEMO_W36_CLEAR_NORMAL, nullptr, nullptr, 200);
            }
        }
    }

    mModel.setPriorityDraw(0, 0);
    return SUCCEEDED;
}

int daWmCourse_c::execute() {
    processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);

    updateState();

    if ((u32)dCsvData_c::c_START_ID != mParam) {
        daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    }

    calcModel();
    updateHelpFade();

    mMatClrAnim[mCurrentIndex].play();
    mModel.play();

    return SUCCEEDED;
}

int daWmCourse_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmCourse_c::doDelete() {
    return SUCCEEDED;
}

void daWmCourse_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("cobCourse", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("cobCourse");

    mModel.create(resMdl, &mAllocator, 0x128, 1, nullptr);

    nw4r::g3d::ResMdl matResMdl = mModel.getResMdl();

    for (int i = 0; i < ANIM_COUNT; i++) {
        nw4r::g3d::ResAnmClr resAnmClr = mResFile.GetResAnmClr(sResAnmNames[i]);
        mMatClrAnim[i].create(matResMdl, resAnmClr, &mAllocator, nullptr, 1);
        mMatClrAnim[i].setRate(0.0f, 0);
        mMatClrAnim[i].setFrame(0.0f, 0);
        mMatClrAnim[i].setPlayMode(sPlayModes[i], 0);
    }

    int courseNo = ACTOR_PARAM(CourseNo);
    int courseType = dWmLib::GetCourseTypeFromCourseNo(courseNo);
    mCurrentIndex = 0xff;

    // `lbl_2_bss_FD7C` decodes (via the REL's relocation stream: exactly four
    // references in the whole module, a write pair in __sinit and a read pair
    // here) to dWmLib::c_StartPointKinokoHouseID -- an EXISTING header static
    // (d_wm_lib.hpp, namespace-scope, `= dCsvData_c::c_START_ID`) that every
    // TU including d_wm_lib.hpp gets its own copy of. Misleadingly named for
    // whatever caller elsewhere actually uses it for kinoko-house purposes,
    // but its value is dCsvData_c::c_START_ID, matching execute()'s direct
    // `dCsvData_c::c_START_ID != mParam` check (already matched) -- same
    // sentinel, compared through the header's cached copy here instead of
    // the constant directly, which is why the two sites disassemble
    // differently even though they test the same value.
    //
    // Every other actor falls into the GetOpenStatus()/GetClearStatus()
    // dispatch. Case-label order below follows the target's own body layout
    // (case 0, case 1, case 2/3, default -- ascending), even though the
    // dispatch COMPARES the case-2/3 range first; MWCC tests grouped-value
    // cases before singletons regardless of declaration order.
    if (mParam != (u32)dWmLib::c_StartPointKinokoHouseID) {
        switch (GetOpenStatus()) {
            case 0:
                setMatClrAnm(0, 0.0f, 0.0f);
                break;
            case 1:
                setMatClrAnm(0, 0.0f, 0.0f);
                break;
            case 2:
            case 3:
                switch (GetClearStatus()) {
                    case 0:
                        setMatClrAnm(2, 1.0f, 0.0f);
                        if (dWmLib::IsCourseUraOtasukeClearSimple(dScWMap_c::m_WorldNo, courseNo)) {
                            setMatClrAnm(1, 1.0f, 0.0f);
                        }
                        break;
                    case 1:
                    case 2:
                    case 3:
                        if (courseType == 4) {
                            setMatClrAnm(0, 0.0f, 0.0f);
                        } else {
                            setMatClrAnm(0, 0.0f, 1.0f);
                        }
                        break;
                    case 4:
                        setMatClrAnm(1, 1.0f, 0.0f);
                        break;
                    default:
                        if (dWmLib::IsCourseUraOtasukeClearSimple(dScWMap_c::m_WorldNo, courseNo)) {
                            setMatClrAnm(1, 1.0f, 0.0f);
                        }
                        break;
                }
                break;
            default:
                setMatClrAnm(0, 0.0f, 0.0f);
                break;
        }
    } else {
        if (!IsCourseClear()) {
            setMatClrAnm(2, 1.0f, 0.0f);
        } else {
            setMatClrAnm(0, 0.0f, 0.0f);
        }
    }

    if (dWmLib::IsAllComplete()) {
        if (dWmLib::GetCourseTypeFromCourseNo(courseNo) == 4) {
            setMatClrAnm(0, 0.0f, 1.0f);
        }
    }

    if (dScWMap_c::m_WorldNo == 0 && courseNo == 0x28) {
        setMatClrAnm(0, 0.0f, 1.0f);
    }

    if (dWmLib::isSpecialWorld()) {
        updateSpecialWorld();
    }

    updateClearAnim(false);

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmCourse_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmCourse_c::updateState() {
    switch (mState) {
        case 0:
            if (mUnk23c) {
                mState = 1;
            }
            break;
        case 1:
            mState = 2;
            break;
        case 2:
            mState = 3;
            break;
        default:
            break;
    }
}

// Matches fn_2_160F50's shape. Two open items, both documented at the point
// of use below: the dInfo_c byte at +0x380 falls inside that class's still
// -unnamed pad11 (real header not editable from here), read via a raw offset
// cast instead of a named member; and the vtable slot the "call through the
// vtable" sites use was pinned down with check_vtable.py against the
// target's own vtable dump (lbl_2_data_444A0) -- offset +0x68 is
// setCutEnd__14dWmDemoActor_cFv, NOT vf74/vf78 as first guessed. The default
// case's "stb 0x139" is the SAME field (dWmDemoActor_c::mIsCutEnd) written
// directly rather than through the vtable, so it is a plain field write here
// instead of a virtual setCutEnd() call.
void daWmCourse_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) {
        return;
    }

    if (isFirstFrame) {
        switch (cutsceneCommandId) {
            case 0x5e:
                mUnk248 = 0;
                if (isWorld2SpecialType()) {
                    if (!*reinterpret_cast<u8 *>(reinterpret_cast<char *>(dInfo_c::m_instance) + 0x380)
                        || IsCourseOmoteClear()) {
                        mUnk248 = 0xa;
                    }
                }
                break;
            case 0x93:
                mUnk248 = 0;
                if (isWorld2SpecialType()) {
                    mUnk248 = 0x1f;
                }
                if (!*reinterpret_cast<u8 *>(reinterpret_cast<char *>(dInfo_c::m_instance) + 0x380)) {
                    mUnk248 = 0;
                }
                if (IsCourseOmoteClear()) {
                    mUnk248 = 0;
                }
                break;
            default:
                break;
        }
    }

    switch (cutsceneCommandId) {
        case 3:
            if (updateOpenAnim()) {
                setCutEnd();
            }
            break;
        case 0x5e:
            if (mUnk248 > 0) {
                if (mUnk248 == 1) {
                    updateClearAnim(true);
                }
                mUnk248 -= 1;
            } else {
                setCutEnd();
            }
            break;
        case 0x93:
            if (mUnk248 > 0) {
                if (mUnk248 == 1
                    && *reinterpret_cast<u8 *>(reinterpret_cast<char *>(dInfo_c::m_instance) + 0x380)) {
                    updateClearAnim(true);
                    fn_80103420(dWmEffectManager_c::m_pInstance, 0x21, mModel, "cobCourse", 0, 0);
                    dWmSeManager_c::m_pInstance->playSound(0x1f, mPos, 1);
                    mUnk250 = true;
                }
                mUnk248 -= 1;
            } else {
                setCutEnd();
            }
            break;
        default:
            mIsCutEnd = true;
            break;
    }
}

void daWmCourse_c::setMatClrAnm(int index, float rate, float frame) {
    mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_MATCLR);
    mModel.setAnm(mMatClrAnim[index]);
    mMatClrAnim[index].setRate(rate, 0);
    mMatClrAnim[index].setFrame(frame, 0);
    mCurrentIndex = index;
}

// TODO: fn_2_191BF0 is an unnamed function INSIDE d_basesNP itself
// (.text:0x191BF0), owned by a TU that is not yet landed. syms.txt only
// carries DOL (0x8xxxxxxx) addresses, so this REL-internal symbol cannot
// resolve at link time until that TU lands -- same class of blocker as
// d_a_wm_kinoko_1up.cpp. Left in, correct and named, deliberately not
// stubbed: a stub would be wrong bytes and would hide the dependency.
//
// Return type corrected to bool: the target initialises r31=0 at entry and
// returns it in r3, setting it to 1 on the "gate failed" early-out and on
// case 4 only -- every other exit (case 0/1/3/default) returns the initial
// 0. This function is not called anywhere visible in this TU; the true
// caller (a state-function-pointer table elsewhere) is out of view, but the
// target's own bytes are unambiguous about the return value existing.
// The world==7/courseNo==0x17/fn_2_191BF0() gate is NOT a side-effect-only
// call as previously assumed -- when GetOpenStatus() != 1 the target skips
// the switch entirely and returns true unless ALL three gate conditions
// hold, in which case it falls into the switch below.
bool daWmCourse_c::updateOpenAnim() {
    bool result = false;
    if (GetOpenStatus() == 1
        || (dScWMap_c::m_WorldNo == 7 && (int)ACTOR_PARAM(CourseNo) == 0x17 && fn_2_191BF0())) {
        switch (mOpenState) {
            case 0:
                mOpenState = 1;
                break;
            case 1:
                fn_80103420(dWmEffectManager_c::m_pInstance, 0x21, mModel, "cobCourse", 0, 0);
                dWmSeManager_c::m_pInstance->playSound(0x1f, mPos, 1);
                setMatClrAnm(2, 1.0f, 0.0f);
                mOpenState = 3;
                break;
            case 3: {
                int frames = 60;
                if (mMatClrAnim[ANM_OPEN].checkFrame(frames, 0)) {
                    openNeighbors(true);
                    mOpenState = 4;
                }
                break;
            }
            case 4:
                result = true;
                break;
            default:
                break;
        }
    } else {
        result = true;
    }
    return result;
}

// dWmLib::SearchMapObjFromCsvIndex loop matching fn_2_161390's shape.
daWmCourse_c *daWmCourse_c::searchOpenNeighbor() {
    for (int i = 0; i < 0xc0; i++) {
        dWmObjActor_c *obj = (dWmObjActor_c *)dWmLib::SearchMapObjFromCsvIndex(0x27e, i);
        if (obj == nullptr) {
            continue;
        }
        if (((int)ACTOR_PARAM_LOCAL(obj->mParam, CourseNo) == 0x28)) {
            continue;
        }
        int status = obj->GetOpenStatus();
        if (status < 2 || status > 3) {
            continue;
        }
        if (obj->IsCourseClear()) {
            continue;
        }
        return (daWmCourse_c *)obj;
    }
    return nullptr;
}

// TODO: one branch (world==7 && kind==0x17) calls fn_2_191BF0, an unnamed,
// unlanded external function outside course's own .text -- cannot be called
// by name yet. Everything else matched against the raw target bytes.
void daWmCourse_c::openNeighbors(bool fastRate) {
    bool any = false;
    for (int i = 0; i < 0xc0; i++) {
        dWmObjActor_c *obj = (dWmObjActor_c *)dWmLib::SearchMapObjFromCsvIndex(0x27e, i);
        if (obj == nullptr) {
            continue;
        }
        if ((int)ACTOR_PARAM_LOCAL(obj->mParam, CourseNo) == 0x28) {
            continue;
        }
        if (obj->GetOpenStatus() == 1
            || (dScWMap_c::m_WorldNo == 7 && (int)ACTOR_PARAM_LOCAL(obj->mParam, CourseNo) == 0x17 && fn_2_191BF0())) {
            any = true;
        }
    }

    if (!any) {
        return;
    }

    daWmCourse_c *neighbor = (daWmCourse_c *)fManager_c::searchBaseByProfName(0x27e, nullptr);
    while (neighbor != nullptr) {
        if (fastRate) {
            neighbor->mMatClrAnim[ANM_OPEN].setRate(1.0f, 0);
        } else {
            neighbor->mMatClrAnim[ANM_OPEN].setRate(0.0f, 0);
            int frames = 60;
            neighbor->mMatClrAnim[ANM_OPEN].setFrame(frames, 0);
        }
        neighbor = (daWmCourse_c *)fManager_c::searchBaseByProfName(0x27e, neighbor);
    }
}

float daWmCourse_c::getMatClrFrame() {
    return mMatClrAnim[ANM_OPEN].getFrame(0);
}

// dWmLib::isSpecialWorldCourseOpen-gated fallback, matches fn_2_161590.
void daWmCourse_c::updateSpecialWorld() {
    u32 courseNo = ACTOR_PARAM(CourseNo);
    if (courseNo <= 9 && !dWmLib::isSpecialWorldCourseOpen(courseNo)) {
        setMatClrAnm(0, 0.0f, 0.0f);
    }
}

// Matches fn_2_1615F0's shape. Gated on courseNo==3 && m_WorldNo==2 (a
// world-2-course-3-specific clear animation), then branches again on the
// same dInfo_c +0x380 byte seen in processCutsceneCommand -- Omote/Otasuke
// guards when set, Ura/UraOtasuke guards when clear. `immediate` selects
// which of the two fixed rates (0.0f/1.0f) feeds the fallback branch; the
// two `IsCourse*Simple()==true` branches always use the fixed pair
// unconditionally.
void daWmCourse_c::updateClearAnim(bool immediate) {
    u32 courseNo = ACTOR_PARAM(CourseNo);
    float baseRate = immediate ? 0.0f : 1.0f;

    if (courseNo == 3 && dScWMap_c::m_WorldNo == 2) {
        if (*reinterpret_cast<u8 *>(reinterpret_cast<char *>(dInfo_c::m_instance) + 0x380)) {
            if (IsCourseOmoteClearSimple()) {
                setMatClrAnm(0, 0.0f, 1.0f);
            } else if (IsCourseOtasukeClearSimple()) {
                setMatClrAnm(1, 1.0f, 0.0f);
            } else {
                int frame = 0;
                setMatClrAnm(2, baseRate, frame);
            }
        } else {
            if (IsCourseUraClearSimple()) {
                setMatClrAnm(0, 0.0f, 1.0f);
            } else if (IsCourseUraOtasukeClearSimple()) {
                setMatClrAnm(1, 1.0f, 0.0f);
            } else {
                int frame = 0;
                setMatClrAnm(2, baseRate, frame);
            }
        }
    }
}

// mUnk250-gated crossfade back to the base rate, matches fn_2_161790.
void daWmCourse_c::updateHelpFade() {
    if (!mUnk250) {
        return;
    }
    daWmCourse_c *neighbor = searchOpenNeighbor();
    if (neighbor != nullptr) {
        float neighborFrame = neighbor->getMatClrFrame();
        float ownFrame = mMatClrAnim[ANM_OPEN].getFrame(0);
        if (ownFrame == neighborFrame) {
            mMatClrAnim[ANM_OPEN].setRate(1.0f, 0);
            mUnk250 = false;
        }
        return;
    }
    mMatClrAnim[ANM_OPEN].setRate(1.0f, 0);
    mUnk250 = false;
}

// Matches fn_2_161840, previously missing entirely from this draft.
bool daWmCourse_c::isWorld2SpecialType() {
    bool result = false;
    if ((int)ACTOR_PARAM(CourseNo) == 3) {
        if (dScWMap_c::m_WorldNo == 2) {
            result = true;
        }
    }
    return result;
}

bool daWmCourse_c::vf78() {
    return mState == 3;
}
