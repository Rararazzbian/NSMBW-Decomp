#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_start.hpp>
#include <game/bases/d_wm_lib.hpp>

ACTOR_PROFILE(WM_START, daWmStart_c, 0);

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
        u8 zoromeTime = dWmLib::getZoromeTime();
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
        int idx = *(int *) ((u8 *) wmMap + 0x338c);
        setStartKinokoShadow__13dWmMapModel_cFb((u8 *) wmMap + idx * 0xbf8 + 0x1a0, true);

        unsigned long param = dWmLib::c_StartPointKinokoHouseID;

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

    u8 zoromeTime = dWmLib::getZoromeTime();
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
void daWmStart_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
}
