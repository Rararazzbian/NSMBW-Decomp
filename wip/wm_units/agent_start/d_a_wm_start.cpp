#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_start.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_w_camera.hpp>
#include <game/bases/d_wm_effect_manager.hpp>

ACTOR_PROFILE(WM_START, daWmStart_c, 0);

// @unofficial Referenced only by ITS ADDRESS (never an individual element load) from
// processCutsceneCommand's case 0x60/0x62 -- `lis/addi lbl_2_rodata_8FD8` materialising a
// pointer, stored into dWCamera_c's own +0x71c field, matching AGENT_CONTEXT.md's "a literal's
// address is materialised with lis/addi" rule. Exact meaning (a camera ease-curve parameter
// block, going by its use site) is inferred, not confirmed.
static const float sc_CamParams[4] = {0.1f, 12.0f, 1.0f, 0.0f};


daWmStart_c::daWmStart_c() : m_200(false) {}
daWmStart_c::~daWmStart_c() {}

// dWmMapModel_c is currently pure padding (no declared members), so this reaches its
// setStartKinokoShadow(bool) the same way the codebase reaches other undecomposed regions --
// by the mangled symbol name, on a raw computed address matching the target's own
// `lwz r0,0x338c(r3); mulli r0,r0,0xbf8; add r3,r3,r0; addi r3,r3,0x1a0` exactly. 0xbf8 is
// sizeof(dWmMapModel_c) (already confirmed by dWmMapModel_c::mPad[0xbf8]).
extern "C" void setStartKinokoShadow__13dWmMapModel_cFb(void *self, bool b);

int daWmStart_c::create() {
    createModel();
    mClipSphere.set(mPos, 50.0f);
    calcModel();

    daWmMap_c *wmMap = daWmMap_c::m_instance;
    int idx = *(int *) ((u8 *) wmMap + 0x338c);
    setStartKinokoShadow__13dWmMapModel_cFb((u8 *) wmMap + idx * 0xbf8 + 0x1a0, false);

    if (IsCourseClear() && !IsCourseFirstClear()) {
        if (dWmLib::getStartPointKinokoHouseKindNum() != 0) {
            dWmLib::setStartPointKinokoHouseKindNum(0);
        }
    }

    if (ACTOR_PARAM(HasChild)) {
        if (!IsCourseClear() || IsCourseFirstClear()) {
            unk_17A3C0();
        } else {
            if (IsCourseFirstOmoteClear()) {
                mSecondChild = nullptr;
                mVec3_c childPos;
                if (ACTOR_PARAM(Kind) != 0) {
                    const char *nodeName = "s1";
                    nw4r::g3d::ResMdl resMdl = mModel.getResMdl();
                    int nodeId = m3d::getNodeID(resMdl, nodeName);
                    mModel.getNodeWorldMtxMultVecZero(nodeId, *(nw4r::math::VEC3 *) &childPos);
                } else {
                    const char *nodeName = "s0";
                    nw4r::g3d::ResMdl resMdl = mModel.getResMdl();
                    int nodeId = m3d::getNodeID(resMdl, nodeName);
                    mModel.getNodeWorldMtxMultVecZero(nodeId, *(nw4r::math::VEC3 *) &childPos);
                }

                if (dWmLib::isStartPointKinokoHouseStar()) {
                    mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_STAR, this,
                                                         dWmLib::c_StartPointKinokoHouseID, &childPos, nullptr);
                } else if (dWmLib::isStartPointKinokoHouseRed()) {
                    mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_RED, this,
                                                         dWmLib::c_StartPointKinokoHouseID, &childPos, nullptr);
                } else if (dWmLib::isStartPointKinokoHouse1up()) {
                    mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_1UP, this,
                                                         dWmLib::c_StartPointKinokoHouseID, &childPos, nullptr);
                }
            } else {
                mChildActor = nullptr;
                mSecondChild = nullptr;
            }
        }
    } else {
        mChildActor = nullptr;
        mSecondChild = nullptr;
        if (!IsCourseClear()) {
            unk_17A760();
        }
        *(u8 *) ((u8 *) this + 0x124) = 0;
    }

    mUnk1fc = 0;
    return SUCCEEDED;
}

// Called from create() when !IsCourseClear() || IsCourseFirstClear(). Repeats create()'s
// "s0"/"s1" node lookup, then either spawns a kinoko-house child (by dWmLib::getStartPointKinokoHouseKindNum/
// IsCourseFirstClear/getZoromeTime) or a Star/Red/1up child (by dWmLib::isStartPointKinokoHouseStar/Red/1up),
// each paired with a second WM_COURSE (0x27e) child at #mPos. Ends by calling #unk_17A760.
void daWmStart_c::unk_17A3C0() {
    mVec3_c childPos;
    if (ACTOR_PARAM(Kind) != 0) {
        const char *nodeName = "s1";
        nw4r::g3d::ResMdl resMdl = mModel.getResMdl();
        int nodeId = m3d::getNodeID(resMdl, nodeName);
        mModel.getNodeWorldMtxMultVecZero(nodeId, *(nw4r::math::VEC3 *) &childPos);
    } else {
        const char *nodeName = "s0";
        nw4r::g3d::ResMdl resMdl = mModel.getResMdl();
        int nodeId = m3d::getNodeID(resMdl, nodeName);
        mModel.getNodeWorldMtxMultVecZero(nodeId, *(nw4r::math::VEC3 *) &childPos);
    }

    if (dWmLib::getStartPointKinokoHouseKindNum() == 0 && !IsCourseFirstClear()) {
        int zoromeTime = dWmLib::getZoromeTime();
        m_1ec = true;

        daWmMap_c *wmMap = daWmMap_c::m_instance;
        int idx = *(int *) ((u8 *) wmMap + 0x338c);
        setStartKinokoShadow__13dWmMapModel_cFb((u8 *) wmMap + idx * 0xbf8 + 0x1a0, false);

        unsigned long param = dWmLib::c_StartPointKinokoHouseID;
        unsigned long paramFlag = param | 0x10000;

        if (zoromeTime == 0) {
            if (dWmLib::IsSingleEntry()) {
                mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_1UP, this, paramFlag, &childPos, nullptr);
                mSecondChild = dWmActor_c::construct(fProfile::WM_COURSE, this, param, &mPos, nullptr);
                *(u8 *) ((u8 *) mChildActor + 0x124) = 0;
                *(u8 *) ((u8 *) mSecondChild + 0x124) = 0;
            } else {
                mChildActor = nullptr;
                mSecondChild = nullptr;
            }
        } else if (zoromeTime == 9) {
            mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_STAR, this, paramFlag, &childPos, nullptr);
            mSecondChild = dWmActor_c::construct(fProfile::WM_COURSE, this, param, &mPos, nullptr);
            *(u8 *) ((u8 *) mChildActor + 0x124) = 0;
            *(u8 *) ((u8 *) mSecondChild + 0x124) = 0;
        } else if (zoromeTime >= 3) {
            mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_RED, this, paramFlag, &childPos, nullptr);
            mSecondChild = dWmActor_c::construct(fProfile::WM_COURSE, this, param, &mPos, nullptr);
            *(u8 *) ((u8 *) mChildActor + 0x124) = 0;
            *(u8 *) ((u8 *) mSecondChild + 0x124) = 0;
        } else if (zoromeTime >= 1) {
            mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_1UP, this, paramFlag, &childPos, nullptr);
            mSecondChild = dWmActor_c::construct(fProfile::WM_COURSE, this, param, &mPos, nullptr);
            *(u8 *) ((u8 *) mChildActor + 0x124) = 0;
            *(u8 *) ((u8 *) mSecondChild + 0x124) = 0;
        }
    } else {
        m_1ec = false;

        daWmMap_c *wmMap = daWmMap_c::m_instance;
        unsigned long param = dWmLib::c_StartPointKinokoHouseID;
        int idx = *(int *) ((u8 *) wmMap + 0x338c);
        setStartKinokoShadow__13dWmMapModel_cFb((u8 *) wmMap + idx * 0xbf8 + 0x1a0, true);

        if (dWmLib::isStartPointKinokoHouseStar()) {
            mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_STAR, this, param, &childPos, nullptr);
            mSecondChild = dWmActor_c::construct(fProfile::WM_COURSE, this, param, &mPos, nullptr);
            *(u8 *) ((u8 *) this + 0x124) = 0;
        } else if (dWmLib::isStartPointKinokoHouseRed()) {
            mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_RED, this, param, &childPos, nullptr);
            mSecondChild = dWmActor_c::construct(fProfile::WM_COURSE, this, param, &mPos, nullptr);
            *(u8 *) ((u8 *) this + 0x124) = 0;
        } else if (dWmLib::isStartPointKinokoHouse1up()) {
            mChildActor = dWmActor_c::construct(fProfile::WM_KINOKO_1UP, this, param, &childPos, nullptr);
            mSecondChild = dWmActor_c::construct(fProfile::WM_COURSE, this, param, &mPos, nullptr);
            *(u8 *) ((u8 *) this + 0x124) = 0;
        }
    }

    unk_17A760();
}

// Called from create() when IsCourseClear() is false. Decides the kinoko-house's dance/reaction
// "kind" (dWmLib::setStartPointKinokoHouseKindNum) from dWmLib::getZoromeTime(), split further by
// dWmLib::IsSingleEntry(). Void return -- no path sets r3 before any exit.
void daWmStart_c::unk_17A760() {
    if (dWmLib::getStartPointKinokoHouseKindNum() != 0) {
        return;
    }
    if (IsCourseFirstClear()) {
        return;
    }

    int zoromeTime = dWmLib::getZoromeTime();
    if (zoromeTime == 0) {
        if (dWmLib::IsSingleEntry()) {
            dWmLib::setStartPointKinokoHouseKindNum(6);
            m_200 = true;
        }
    } else if (zoromeTime == 9) {
        if (!dWmLib::IsSingleEntry()) {
            dWmLib::setStartPointKinokoHouseKindNum(4);
        } else {
            dWmLib::setStartPointKinokoHouseKindNum(1);
        }
        m_200 = true;
    } else if (zoromeTime >= 3) {
        if (!dWmLib::IsSingleEntry()) {
            dWmLib::setStartPointKinokoHouseKindNum(5);
        } else {
            dWmLib::setStartPointKinokoHouseKindNum(2);
        }
        m_200 = true;
    } else if (zoromeTime >= 1) {
        if (!dWmLib::IsSingleEntry()) {
            dWmLib::setStartPointKinokoHouseKindNum(6);
        } else {
            dWmLib::setStartPointKinokoHouseKindNum(3);
        }
        m_200 = true;
    }
}

int daWmStart_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    calcModel();
    mAnimTexPat.play();
    return SUCCEEDED;
}

int daWmStart_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmStart_c::doDelete() {
    return SUCCEEDED;
}

void daWmStart_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);
    mResFile = dResMng_c::m_instance->getRes("cobStart", "g3d/model.brres");

    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("cobStart");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    nw4r::g3d::ResAnmTexPat resAnmTexPat = mResFile.GetResAnmTexPat("cobStart");
    mAnimTexPat.create(resMdl, resAnmTexPat, &mAllocator, 1);
    mModel.setAnm(mAnimTexPat);

    mAnimTexPat.setRate(0.0f, 0);
    int kind = ACTOR_PARAM(Kind);
    if (kind == 0) {
        mAnimTexPat.setFrame(0.0f, 0);
    } else if (kind == 1) {
        mAnimTexPat.setFrame(1.0f, 0);
    }

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmStart_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

// NOT YET AUTHORED this round (0x3f4 bytes, the largest function in the unit).
// dWCamera_c's header still models the wrong thing (see source/d_basesNP/bases/d_a_wm_note.cpp,
// which shipped today and uses the same technique: a local u8* cast confined to this .cpp,
// touching no shared header). Offsets confirmed against these target bytes directly.
extern "C" void playEffect__18dWmEffectManager_cFiPC7mVec3_cPC7mAng3_cPC7mVec3_c(
    void *self, int id, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale);

void daWmStart_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) {
        return;
    }

    dWCamera_c *camera = dWCamera_c::m_instance;
    daWmMap_c *wmMap = daWmMap_c::m_instance;

    if (isFirstFrame) {
        switch (cutsceneCommandId) {
        case 0x60:
            if (mSecondChild != nullptr && m_1ec) {
                // Player-derived world position, fetched through an undeclared daWmPlayer_c
                // member (ms_instance+0x29c is itself a pointer, dereferenced twice more before
                // the call -- replicated mechanically, not modelled by name).
                u8 *playerObj = *(u8 **) ((u8 *) daWmPlayer_c::ms_instance + 0x29c);
                u8 *table = *(u8 **) (playerObj + 4);
                typedef void (*GetPosFn)(mVec3_c *out, void *self);
                GetPosFn fn = *(GetPosFn *) (table + 0x30);
                fn(&mCamTarget, playerObj);

                *(u32 *) ((u8 *) camera + 0x604) = 2;
                *(mVec3_c **) ((u8 *) camera + 0x5f4) = &mCamTarget;
                *(u32 *) ((u8 *) camera + 0x5f0) = 0;
                *(bool *) ((u8 *) camera + 0x624) = false;
                *(u32 *) ((u8 *) camera + 0x608) = 0;
                *(const float **) ((u8 *) camera + 0x71c) = sc_CamParams;
                mUnk1fc = 0x1e;
            } else {
                setCutEnd();
            }
            break;
        case 0x61:
            if (mSecondChild != nullptr && m_1ec) {
                *(u8 *) ((u8 *) this + 0x124) = false;
                bool flag = true;
                *(u8 *) ((u8 *) mSecondChild + 0x124) = flag;

                dWmEffectManager_c *effMgr = dWmEffectManager_c::m_pInstance;
                playEffect__18dWmEffectManager_cFiPC7mVec3_cPC7mAng3_cPC7mVec3_c(
                    effMgr, 0x21, (const mVec3_c *) ((u8 *) mSecondChild + 0xac), nullptr, nullptr);

                *(u8 *) ((u8 *) mChildActor + 0x124) = flag;
                mUnk1fc = 0;
            }
            break;
        case 0x3c:
            if (mSecondChild != nullptr && !m_1ec && IsCourseFirstClear()) {
                *(u8 *) ((u8 *) this + 0x124) = true;
                *(u8 *) ((u8 *) mSecondChild + 0x124) = false;
                mUnk1fc = 0;
            }
            break;
        case 0x62:
            if (mSecondChild != nullptr && m_1ec) {
                mVec3_c localTemp; // uninitialised in the target -- no store before its address is taken
                *(u32 *) ((u8 *) camera + 0x604) = 7;
                *(mVec3_c **) ((u8 *) camera + 0x5f4) = &localTemp;
                *(u32 *) ((u8 *) camera + 0x5f0) = 0;
                *(bool *) ((u8 *) camera + 0x624) = false;
                *(u32 *) ((u8 *) camera + 0x608) = 0;
                *(const float **) ((u8 *) camera + 0x71c) = sc_CamParams;
                mUnk1fc = 0x1e;
            } else {
                setCutEnd();
            }
            break;
        }
    } else {
    switch (cutsceneCommandId) {
    case 0x60:
        if (*(u32 *) ((u8 *) camera + 0x5f4) != 0) {
            return;
        }
        if (mUnk1fc > 0) {
            mUnk1fc--;
        } else {
            setCutEnd();
        }
        break;
    case 0x61:
        if (mSecondChild != nullptr && m_1ec) {
            if (mUnk1fc > 0) {
                mUnk1fc--;
            } else {
                int idx = *(int *) ((u8 *) wmMap + 0x338c);
                setStartKinokoShadow__13dWmMapModel_cFb((u8 *) wmMap + idx * 0xbf8 + 0x1a0, true);
                setCutEnd();
            }
        } else {
            setCutEnd();
        }
        break;
    case 0x3c:
        if (mSecondChild != nullptr && IsCourseFirstClear()) {
            if (mUnk1fc > 0) {
                mUnk1fc--;
            } else {
                int idx = *(int *) ((u8 *) wmMap + 0x338c);
                setStartKinokoShadow__13dWmMapModel_cFb((u8 *) wmMap + idx * 0xbf8 + 0x1a0, false);
                setCutEnd();
            }
        } else {
            setCutEnd();
        }
        break;
    case 0x62: {
        bool arrived = false;
        if (*(u32 *) ((u8 *) camera + 0x5f4) == 0) {
            if (*(float *) ((u8 *) camera + 0x4ec) == *(float *) ((u8 *) camera + 0x4e0) &&
                *(float *) ((u8 *) camera + 0x4f0) == *(float *) ((u8 *) camera + 0x4e4) &&
                *(float *) ((u8 *) camera + 0x4f4) == *(float *) ((u8 *) camera + 0x4e8)) {
                arrived = true;
            }
        }
        if (!arrived) {
            return;
        }
        if (mUnk1fc > 0) {
            mUnk1fc--;
        } else {
            setCutEnd();
        }
        break;
    }
    default:
        *(bool *) ((u8 *) this + 0x139) = true;
        break;
    }
    }
}
