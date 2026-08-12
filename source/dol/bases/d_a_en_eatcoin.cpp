#include <game/bases/d_a_en_eatcoin.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_coin.hpp>
#include <game/bases/d_res_mng.hpp>
#include <constants/sound_list.h>

ACTOR_PROFILE(EN_EATCOIN, daEnEatCoin_c, 2);

STATE_VIRTUAL_DEFINE(daEnEatCoin_c, EatOut);

static const char *l_eatcoin_modeldt[] = {"obj_coin", "obj_coin_blue", "obj_coin_red"};

int daEnEatCoin_c::create() {
    mCoinType = ACTOR_PARAM(Type);

    model_set();

    mEatBehavior = EAT_TYPE_DRINK;

    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

    mAngle.y = dCoin_c::getShapeAngle().y;

    changeState(StateID_EatIn);

    return SUCCEEDED;
}

int daEnEatCoin_c::execute() {
    mStateMgr.executeState();
    return SUCCEEDED;
}

int daEnEatCoin_c::draw() {
    mVec3_c pos = mPos;
    mVec3_c scale = mScale;
    mAng3_c angle = mAngle;

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.YrotM(angle.y);
    mMatrix.XrotM(angle.x);
    mMatrix.ZrotM(angle.z);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(scale);
    mModel.calc(false);
    mModel.entry();

    return SUCCEEDED;
}

int daEnEatCoin_c::doDelete() {
    return SUCCEEDED;
}

bool daEnEatCoin_c::setEatGlupDown(dActor_c *eatingActor) {
    s8 plrNo = eatingActor->getPlrNo();

    daPyMng_c::incCoin(plrNo);
    daPyMng_c::addScore(100, plrNo);

    dAudio::SoundEffectID_t(SE_OBJ_GET_COIN).playMapSound(mPos, dAudio::getRemotePlayer(plrNo));

    deleteRequest();

    return true;
}

void daEnEatCoin_c::initializeState_EatOut() {
    deleteRequest();
}

void daEnEatCoin_c::finalizeState_EatOut() {}

void daEnEatCoin_c::executeState_EatOut() {}

void daEnEatCoin_c::model_set() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("obj_coin", "g3d/obj_coin.brres");
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(l_eatcoin_modeldt[mCoinType]);
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC);
    dActor_c::setSoftLight_MapObj(mModel);

    mAllocator.adjustFrmHeap();
}

daEnEatCoin_c::~daEnEatCoin_c() {}
