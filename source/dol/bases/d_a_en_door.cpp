#include <game/bases/d_a_en_door.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/framework/f_manager.hpp>
#include <game/mLib/m_allocator_dummy_heap.hpp>
#include <game/mLib/m_effect.hpp>
#include <constants/sound_list.h>

STATE_DEFINE(daEnDoor_c, Search);
STATE_DEFINE(daEnDoor_c, Open);
STATE_DEFINE(daEnDoor_c, Close);
STATE_DEFINE(daEnDoor_c, Wait);
STATE_DEFINE(daEnDoor_c, Dummy);

dActor_c *daEnDoor_c::searchParent_centerA(u32 no) {
    daObjCenter_c *center = nullptr;
    while ((center = (daObjCenter_c *) fManager_c::searchBaseByProfName(fProfile::OBJ_CENTER, center)) != nullptr) {
        if (no == center->mNo) {
            return center;
        }
    }
    return nullptr;
}

dActor_c *daEnDoor_c::searchParent_centerB(u32 no) {
    daObjCenter2_c *center = nullptr;
    while ((center = (daObjCenter2_c *) fManager_c::searchBaseByProfName(fProfile::OBJ_CENTER2, center)) != nullptr) {
        if (no == center->mNo) {
            return center;
        }
    }
    return nullptr;
}

dActor_c *daEnDoor_c::searchParent(u32 no) {
    dActor_c *center = searchParent_centerA(no);
    if (center != nullptr) {
        return center;
    }
    return searchParent_centerB(no);
}

void daEnDoor_c::setParentInfo(dActor_c *parent) {
    mParentID = parent->mUniqueID;
    mParentOffset = mPos - parent->mPos;
}

void daEnDoor_c::rotation_move(dActor_c *parent) {
    mAng angle = parent->mAngle.z;
    angle += 0x4000;
    mVec3_c pos = parent->mPos;

    mAngle.z = angle.mAngle + mBaseAngle;

    float cos = nw4r::math::CosIdx(angle);
    float sin = nw4r::math::SinIdx(angle);
    float ox = mParentOffset.x;
    float oy = mParentOffset.y;

    mPos.x = pos.x + (ox * cos - oy * sin);
    mPos.y = pos.y + (ox * sin + oy * cos);
}

dActor_c *daEnDoor_c::rotation_move() {
    dActor_c *parent = (dActor_c *) fManager_c::searchBaseByID(mParentID);
    if (parent != nullptr) {
        rotation_move(parent);
    }
    return parent;
}

dActor_c *daEnDoor_c::setParentInfo(u32 no) {
    dActor_c *parent = searchParent(no);
    if (parent != nullptr) {
        setParentInfo(parent);
        rotation_move(parent);
        changeState(StateID_Wait);
    }
    return parent;
}

int daEnDoor_c::create() {
    allocate();

    mScale.set(1.0f, 1.0f, 1.0f);
    mCenterOffs.set(0.0f, 24.0f, 0.0f);

    mVisibleAreaOffset.set(0.0f, 24.0f);
    mVisibleAreaSize.set(32.0f, 48.0f);
    mMaxBound.mOffset.set(smc_CULL_XLIMIT, smc_CULL_YLIMIT);

    initCcData();
    mExecStopMask &= ~BIT_FLAG(STAGE_ACTOR_ENEMY);
    initAnm();
    initialize();

    u32 param = mParam;

    mPos.z = 32.0f;
    mParentID = (fBaseID_e) 0;
    mBaseAngle = ACTOR_PARAM_LOCAL(param, Direction) * 0x4000;
    u8 hasParent = ACTOR_PARAM_LOCAL(param, HasParent);
    mHasParent = hasParent;

    if (hasParent) {
        mParentNo = ACTOR_PARAM_LOCAL(param, ParentNo);
        if (setParentInfo(ACTOR_PARAM_LOCAL(param, ParentNo)) == nullptr) {
            changeState(StateID_Search);
        }
    } else {
        changeState(StateID_Wait);
    }

    return SUCCEEDED;
}

void daEnDoor_c::initCcData() {}

void daEnDoor_c::initAnm() {}

void daEnDoor_c::initialize() {
    mCc.entry();
}

void daEnDoor_c::allocate() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);
    createMdl();
    mAllocator.adjustFrmHeap();
}

void daEnDoor_c::createMdl() {}

int daEnDoor_c::execute() {
    if (mHasParent) {
        if (mParentID != 0 && rotation_move() == nullptr) {
            deleteActor(0);
            return SUCCEEDED;
        }
    } else if (ActorScrOutCheck(0)) {
        return SUCCEEDED;
    }

    mStateMgr.executeState();
    calcMdl();

    return SUCCEEDED;
}

int daEnDoor_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daEnDoor_c::doDelete() {
    if (mAllocator.mpHeap != mAllocatorDummyHeap_c::getInstance()) {
        mAnmChr.remove();
        mModel.remove();
    }
    return SUCCEEDED;
}

void daEnDoor_c::calcMdl() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos);
    mMatrix.YrotM(angle.y);
    mMatrix.XrotM(angle.x);
    mMatrix.ZrotM(angle.z);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daEnDoor_c::ccCallback(dCc_c *self, dCc_c *other) {
    dActor_c *player = other->mpOwner;
    if (player->mKind != STAGE_ACTOR_PLAYER) {
        return;
    }

    daEnDoor_c *door = (daEnDoor_c *) self->mpOwner;
    if (door->isDummyOpen()) {
        if (!((dAcPy_c *) player)->isDoorDemoEnable()) {
            return;
        }
        if (door->isClosed()) {
            door->changeState(StateID_Dummy);
        }
        self->mInfo |= CC_NO_HIT;
    } else if (door->checkOpenOk()) {
        if (((dAcPy_c *) player)->setDoorDemo(door)) {
            door->mDemoActive = 1;
            self->mInfo |= CC_NO_HIT;
        }
    }
}

bool daEnDoor_c::isDummyOpen() {
    return false;
}

bool daEnDoor_c::isClosed() {
    return isState(StateID_Wait);
}

bool daEnDoor_c::checkOpenOk() {
    if (isClosed()) {
        return true;
    }
    if (isState(StateID_Open) && mOpenType == 1) {
        return true;
    }
    return false;
}

void daEnDoor_c::initializeState_Search() {}

void daEnDoor_c::finalizeState_Search() {}

void daEnDoor_c::executeState_Search() {
    setParentInfo(mParentNo);
}

void daEnDoor_c::initializeState_Open() {
    setOpenAnm();
    setOpenSE();
}

void daEnDoor_c::setOpenAnm() {}

void daEnDoor_c::setOpenSE() {}

void daEnDoor_c::finalizeState_Open() {
    mIsOpen = 0;
}

void daEnDoor_c::executeState_Open() {
    mModel.play();
    if (mAnmChr.isStop()) {
        mIsOpen = 1;
        if (mOpenType == 3) {
            changeState(StateID_Close);
        }
    }
}

void daEnDoor_c::initializeState_Dummy() {
    dAudio::SoundEffectID_t(SE_OBJ_DDOOR_OPEN).playMapSound(mPos, 0);

    mVec3_c effPos = getCenterPos();
    effPos.z = 2800.0f;
    mEf::createEffect("Wm_en_obakedoor", 0, &effPos, nullptr, nullptr);

    mVec3_c coinPos = getCenterPos();
    dActorMng_c::m_instance->createUpCoin(coinPos, 0, 1, 0);
}

void daEnDoor_c::finalizeState_Dummy() {}

void daEnDoor_c::executeState_Dummy() {
    deleteActor(1);
}

void daEnDoor_c::initializeState_Close() {
    setCloseAnm();
}

void daEnDoor_c::setCloseAnm() {}

void daEnDoor_c::finalizeState_Close() {}

void daEnDoor_c::executeState_Close() {
    mModel.play();
    setCloseMoveSE();
    if (mAnmChr.isStop()) {
        setCloseSE();
        changeState(StateID_Wait);
    }
}

void daEnDoor_c::setCloseMoveSE() {}

void daEnDoor_c::setCloseSE() {}

void daEnDoor_c::initializeState_Wait() {
    mOpenType = 0;
    mDemoActive = 0;
    setWaitAnm();
}

void daEnDoor_c::setWaitAnm() {}

void daEnDoor_c::finalizeState_Wait() {}

void daEnDoor_c::executeState_Wait() {
    mModel.play();
    waitProc();
    if (mDemoActive != 0 && (mOpenType == 1 || mOpenType == 2)) {
        changeState(StateID_Open);
    }
}

void daEnDoor_c::waitProc() {}

bool daEnDoor_c::ActorDrawCullCheck() {
    return false;
}

daEnDoor_c::~daEnDoor_c() {}
