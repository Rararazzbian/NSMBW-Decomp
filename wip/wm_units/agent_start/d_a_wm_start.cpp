#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_start.hpp>
#include <game/bases/d_wm_lib.hpp>

ACTOR_PROFILE(WM_START, daWmStart_c, 0);

daWmStart_c::daWmStart_c() : m_200(false) {}
daWmStart_c::~daWmStart_c() {}

// NOT YET AUTHORED this round (0x24c bytes). Read but not written: spawns one of three
// kinoko-house child actors (profile IDs 0x27b/0x27c/0x27d, gated by dWmLib::isStartPointKinokoHouse
// Star/Red/1up) at a node position resolved via getResMdl/getNodeID/getNodeWorldMtxMultVecZero on
// either the "s0" or "s1" named node (lbl_2_data_47490/47494), selected by a bitfield at
// ACTOR_PARAM offset 4. Calls #createModel and #calcModel first, then dWmLib::setStartKinokoShadow,
// then branches through IsCourseClear/IsCourseFirstClear/IsCourseFirstOmoteClear to reach either
// #unk_17A3C0 or #unk_17A760. See this task's report.
int daWmStart_c::create() {
    return SUCCEEDED;
}

// NOT YET AUTHORED this round (0x39c bytes). Called from create() when
// IsCourseClear() && IsCourseFirstClear() are both true.
void daWmStart_c::unk_17A3C0() {
}

// NOT YET AUTHORED this round (0x108 bytes). Called from create() when IsCourseClear() is false.
void daWmStart_c::unk_17A760() {
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
