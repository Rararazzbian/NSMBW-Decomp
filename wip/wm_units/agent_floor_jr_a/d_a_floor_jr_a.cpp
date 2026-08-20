#include <game/bases/d_a_floor_jr_a.hpp>

// FLOOR_JR_A. See the header for the full bounds/member-layout evidence.
// STUB PASS: structural skeleton first (vtable slot count / .ctors count /
// sizeof), function bodies filled in incrementally.

daFloorJrA_c::daFloorJrA_c() {
}

STATE_DEFINE(daFloorJrA_c, DemoWait);
STATE_DEFINE(daFloorJrA_c, Wait);
STATE_VIRTUAL_DEFINE(daFloorJrA_c, DieFall);

ACTOR_PROFILE(FLOOR_JR_A, daFloorJrA_c, 0);

daFloorJrA_c::~daFloorJrA_c() {
}

void daFloorJrA_c::setUnk_674_348(int val674, u8 val348) {
    m_674 = val674;
    *((u8 *)this + 0x348) = val348;
}

int daFloorJrA_c::create() {
    createMdl();
    resetToBasePos();
    changeState(StateID_Wait);
    mStateMgr.refreshState();
    return SUCCEEDED;
}

void daFloorJrA_c::createMdl() {
    mHeapAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("boss_koopaJr_down_asiba", "g3d/boss_koopaJr_down_asiba.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("boss_koopaJr_down_asiba");

    mModel.create(resMdl, &mHeapAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
    setSoftLight_MapObj(mModel);
    mHeapAllocator.adjustFrmHeap();
}

void daFloorJrA_c::resetToBasePos() {
    mBasePos = mPos;
    m_674 = -1;
    setupBgCtr();
}

int daFloorJrA_c::execute() {
    mStateMgr.executeState();
    unk_83B00();
    mBgCtr.calc();
    return SUCCEEDED;
}

void daFloorJrA_c::setupBgCtr() {
    *(float *)((u8 *)this + 0xdc) = 1.0f;
    *(float *)((u8 *)this + 0xe0) = 1.0f;
    *(float *)((u8 *)this + 0xe4) = 1.0f;

    mVec3_c scale;
    scale.x = 1.0f;
    scale.y = 1.0f;
    scale.z = 1.0f;

    sBgSetInfo info;
    info.mLeft = -32.0f;
    info.mTop = 8.0f;
    info.mRight = 32.0f;
    info.mBottom = -8.0f;
    info.m_10 = 0;
    info.m_14 = 0;
    info.m_18 = 0;

    mBgCtr.set(this, &info, 3, *((u8 *)this + 0x38f), &scale);
    mBgCtr.entry();
}

void daFloorJrA_c::unk_83B00() {
}

int daFloorJrA_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daFloorJrA_c::doDelete() {
    mBgCtr.release();
    return SUCCEEDED;
}

void daFloorJrA_c::initializeState_DemoWait() {}
void daFloorJrA_c::executeState_DemoWait() {}
void daFloorJrA_c::finalizeState_DemoWait() {}

void daFloorJrA_c::initializeState_Wait() {}
void daFloorJrA_c::executeState_Wait() {}
void daFloorJrA_c::finalizeState_Wait() {}

void daFloorJrA_c::initializeState_DieFall() {}
void daFloorJrA_c::executeState_DieFall() {}
void daFloorJrA_c::finalizeState_DieFall() {}
