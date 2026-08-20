#include <game/bases/d_a_floor_jr_a.hpp>

// FLOOR_JR_A. See the header for the full bounds/member-layout evidence.
//
// DEFINITION ORDER, ground-truthed against bin/dtk/d_basesNP_symbols.txt
// directly (not the order gate's own pairing output):
//   834B0/83510/835A0/835D0/83600  sFStateID_c<daFloorJrA_c>'s own template
//                                  instantiations (dtor/isSameName/init/exec/
//                                  finalize) -- auto-generated at the point of
//                                  FIRST USE of the template, which is why the
//                                  two STATE_DEFINE calls below come first.
//   83630  classInit
//   83660  ctor
//   836E0  dtor
//   83780  setUnk_674_348
//   83790  create
//   83810  createMdl
//   838C0  resetToBasePos
//   83910  execute
//   83970  setupBgCtr
//   83A10  playCrumbleEffects
//   83A90  unk_83A90 (mEffects[0/1].follow(&mPos, 0, 0))
//   83B00  unk_83B00 (own new virtual, vtable tail slot 2)
//   83C50  draw
//   83C80  doDelete
//   83CB0/83CC0/83CD0  finalizeState_DemoWait / initializeState_DemoWait /
//                      executeState_DemoWait -- confirmed from __sinit's own
//                      copied member-function-pointer fields (init=83CC0,
//                      exec=83CD0, final=83CB0), NOT the usual init/exec/final
//                      text order.
//   83CE0/83CF0/83D00  finalizeState_Wait / initializeState_Wait /
//                      executeState_Wait, same pattern.
//   83D40/83D60/83D70  initializeState_DieFall / finalizeState_DieFall /
//                      executeState_DieFall -- confirmed from the vtable's
//                      own slot order (94/95/96 = init/exec/final) mapped to
//                      addresses 83D40/83D70/83D60, i.e. TEXT order is
//                      init, final, exec.
//   83DE0  __sinit (own split object, auto_fn_2_83DE0_text.o)

STATE_DEFINE(daFloorJrA_c, DemoWait);
STATE_DEFINE(daFloorJrA_c, Wait);
STATE_DEFINE(daFloorJrA_c, DieFall);

ACTOR_PROFILE(FLOOR_JR_A, daFloorJrA_c, 0);

daFloorJrA_c::daFloorJrA_c() {
}

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

int daFloorJrA_c::resetToBasePos() {
    mBasePos = mPos;
    m_674 = -1;
    setupBgCtr();
    return SUCCEEDED;
}

int daFloorJrA_c::execute() {
    mStateMgr.executeState();
    unk_83B00();
    mBgCtr.calc();
    return SUCCEEDED;
}

void daFloorJrA_c::setupBgCtr() {
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

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

void daFloorJrA_c::playCrumbleEffects() {
    mEffects[0].createEffect("Wm_jr_crumble01", 0, &mPos, nullptr, nullptr);
    mEffects[1].createEffect("Wm_jr_crumble02", 0, &mPos, nullptr, nullptr);
}

void daFloorJrA_c::unk_83A90() {
    mEffects[0].follow(&mPos, nullptr, nullptr);
    mEffects[1].follow(&mPos, nullptr, nullptr);
}

void daFloorJrA_c::unk_83B00() {
    mMatrix.trans(mPos);

    mMatrix.YrotM(mAngle.y);
    mMatrix.concat(mMtx_c::createTrans(l_EnMuki[mDirection] * 32.0f, 0.0f, 0.0f));

    mMatrix.XrotM(mAngle.x);
    mMatrix.concat(mMtx_c::createTrans(-l_EnMuki[mDirection] * 32.0f, 0.0f, 0.0f));

    mMatrix.ZrotM(mAngle.z);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
}

int daFloorJrA_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daFloorJrA_c::doDelete() {
    mBgCtr.release();
    return SUCCEEDED;
}

void daFloorJrA_c::finalizeState_DemoWait() {}
void daFloorJrA_c::initializeState_DemoWait() {}
void daFloorJrA_c::executeState_DemoWait() {}

void daFloorJrA_c::finalizeState_Wait() {}
void daFloorJrA_c::initializeState_Wait() {}

void daFloorJrA_c::executeState_Wait() {
    if (m_674 == 0)
        changeState(StateID_DemoWait);
    else if (m_674 > 0)
        m_674--;
}

void daFloorJrA_c::initializeState_DieFall() {
    *(float *)((u8 *)this + 0xec) = 0.0f;
    *(float *)((u8 *)this + 0x114) = -0.15f;
    playCrumbleEffects();
}

void daFloorJrA_c::finalizeState_DieFall() {}

void daFloorJrA_c::executeState_DieFall() {
    calcSpeedY();
    posMove();
    unk_83A90();
    mAngle.z += l_EnMuki[mDirection] * 0x60;
}
