#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_lib.hpp>
#include "d_a_wm_tower.hpp"

ACTOR_PROFILE(WM_TOWER, daWmTower_c, 0);

daWmTower_c::daWmTower_c() : mResNodeIdx(-1) {}
daWmTower_c::~daWmTower_c() {}

int daWmTower_c::create() {
    createModel();
    calcModel();

    mClipSphere.set(mPos, 120.0f);

    return SUCCEEDED;
}

int daWmTower_c::execute() {
    processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);

    daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    calcModel();

    return SUCCEEDED;
}

int daWmTower_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmTower_c::doDelete() {
    return SUCCEEDED;
}

void daWmTower_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobTower", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobTower");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1, nullptr);
    dWmActor_c::setSoftLight_MapObj(mModel);

    mAllocator.adjustFrmHeap();
}

void daWmTower_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}
