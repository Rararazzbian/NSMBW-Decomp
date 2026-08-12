#include <game/bases/d_a_en_dpakkun_base.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/bases/d_effectmanager.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/bases/d_multi_manager.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_score_manager.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/mLib/m_heap.hpp>
#include <constants/sound_list.h>

const sCcDatNewF daEnDpakkunBase_c::smc_cc_dat = {
    0.0f, 8.0f,
    8.0f, 8.0f,
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) |
        BIT_FLAG(CC_KIND_YOSHI) | BIT_FLAG(CC_KIND_ENEMY) |
        BIT_FLAG(CC_KIND_TAMA),
    (u32) ~(BIT_FLAG(CC_ATTACK_NONE) | BIT_FLAG(CC_ATTACK_YOSHI_MOUTH) | BIT_FLAG(CC_ATTACK_SPIN_LIFT_UP) | BIT_FLAG(CC_ATTACK_SAND_PILLAR)),
    BIT_FLAG(1),
    dEn_c::normal_collcheck
};

extern const mVec2_c l_hole_offset[4] = {
    mVec2_c(0.0f, 32.0f),
    mVec2_c(0.0f, -32.0f),
    mVec2_c(48.0f, 0.0f),
    mVec2_c(-32.0f, 0.0f)
};

STATE_VIRTUAL_DEFINE(daEnDpakkunBase_c, Wait);
STATE_VIRTUAL_DEFINE(daEnDpakkunBase_c, Appear);
STATE_VIRTUAL_DEFINE(daEnDpakkunBase_c, Attack);
STATE_VIRTUAL_DEFINE(daEnDpakkunBase_c, Disappear);
STATE_VIRTUAL_DEFINE(daEnDpakkunBase_c, DieIceBreak);
STATE_VIRTUAL_DEFINE(daEnDpakkunBase_c, DieVanish);

int daEnDpakkunBase_c::create() {
    allocate();

    mPos.z = -2500.0f;

    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

    mAccelY = 0.0f;
    mEatBehavior = EAT_TYPE_DRINK;
    mActorProperties |= BIT_FLAG(9);

    mStartPos.x = mPos.x;
    mStartPos.y = mPos.y;
    mStartPos.z = mPos.z;

    mIsDying = 0;

    mIceMng.setIceStatus(0, 1, 1);

    for (int i = 0; i < 9; i++) {
        mNeckAngle[i] = 0;
    }

    mModel.setCallback(&mNodeCallback);

    initialize();
    initPakkunDir();
    changeState(StateID_Wait);
    calcMdl();
    updateCc();

    return SUCCEEDED;
}

void daEnDpakkunBase_c::initialize() {}

void daEnDpakkunBase_c::allocate() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);
    createMdl();
    mAllocator.adjustFrmHeap();
}

void daEnDpakkunBase_c::createMdl() {
    mResFile = dResMng_c::m_instance->getRes("pakkun", "g3d/pakkun.brres");
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::BUFFER_RESTEV, 1, nullptr);
    dActor_c::setSoftLight_Enemy(mModel);
    mAnm.create(mdl, mResFile.GetResAnmChr("dokan_attack"), &mAllocator, nullptr);
}

int daEnDpakkunBase_c::execute() {
    mStateMgr.executeState();

    if (isState(StateID_Wait) || isState(StateID_Appear) || isState(StateID_Attack) || isState(StateID_Disappear)) {
        updateCc();
    }

    ActorScrOutCheck(0);
    return SUCCEEDED;
}

void daEnDpakkunBase_c::postExecute(fBase_c::MAIN_STATE_e status) {
    if (status == SUCCESS) {
        mCc.clear();
    }
    dEn_c::postExecute(status);
}

int daEnDpakkunBase_c::preDraw() {
    if (!dEn_c::preDraw()) {
        return false;
    }
    return !isState(StateID_Wait);
}

int daEnDpakkunBase_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

void daEnDpakkunBase_c::deleteReady() {}

int daEnDpakkunBase_c::doDelete() {
    removeCc();
    return SUCCEEDED;
}

void daEnDpakkunBase_c::calcMdl() {
    mVec3_c pos = mPos;
    mAng3_c angle;
    angle.x = mAngle.x;
    angle.y = mAngle.y;
    angle.z = mAngle.z;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);

    float pivotY = 16.0f;
    if (isState(StateID_DieIceBreak)) {
        pivotY = 8.0f;
    }

    mMatrix.ZrotM(angle.z);

    mMtx_c up;
    up.trans(0.0f, pivotY, 0.0f);
    mMatrix.concat(up);

    mMatrix.ZrotM(mSpinAngle);

    mMtx_c down;
    down.trans(0.0f, -pivotY, 0.0f);
    mMatrix.concat(down);

    mMatrix.XrotM(angle.x);
    mMatrix.YrotM(angle.y);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);

    calcJnt();
}

void daEnDpakkunBase_c::calcJnt() {
    mModel.getResMdl();

    mMtx_c mtx;
    mModel.getNodeWorldMtx(7, &mtx);
    mtx.multVecZero(mNodePos7);

    mModel.getNodeWorldMtx(6, &mtx);
    mtx.multVecZero(mNodePos6);

    mMtx_c ofs;
    ofs.trans(0.0f, 6.0f, 0.0f);
    mtx.concat(ofs);
    mtx.multVecZero(mMouthPos);

    mFirePos.x = mMouthPos.x;
    mFirePos.y = mMouthPos.y;
    mFirePos.z = mMouthPos.z - 48.0f;

    mModel.getNodeWorldMtx(5, &mtx);
    mtx.multVecZero(mNodePos5);
}

BOOL daEnDpakkunBase_c::isQuakeDamage() {
    if (mIsDying) {
        return false;
    }
    if (mNoRespawn) {
        return false;
    }
    if (isState(dEn_c::StateID_Ice)) {
        return false;
    }
    if (isState(StateID_Wait)) {
        return false;
    }
    return checkQuakeDeath();
}

void daEnDpakkunBase_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    if (!Enfumi_check(self, other, 1)) {
        dEn_c::Normal_VsYoshiHitCheck(self, other);
    }
}

bool daEnDpakkunBase_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    if (mIsDying) {
        return true;
    }

    if (ACTOR_PARAM(NoFireDamage)) {
        fireballInvalid(self, other);
        return true;
    }

    daPlBase_c *player = (daPlBase_c *) other->mpOwner;
    mVec3_c pos = mPos;
    pos.x += mCenterOffs.x;
    pos.y += mCenterOffs.y;
    pos = pos; // @reconstruction: forces the (otherwise dead) aggregate to keep an address

    dScoreMng_c *scoreMng;
    u32 plrNo = player->getPlrNo();
    if (plrNo <= 3) {
        scoreMng = dScoreMng_c::m_instance;
        scoreMng->ScoreSet(this, mCombo.getDamageScore(), plrNo);
    }

    dAudio::g_pSndObjEmy->startSound(SE_EMY_DOWN, mPos, 0);

    u8 dir = !(player->mSpeed.x >= 0.0f);
    dActorMng_c::m_instance->createUpCoin(getCenterPos(), dir, 1, 0);

    kill(player);
    return true;
}

bool daEnDpakkunBase_c::hitCallback_YoshiFire(dCc_c *self, dCc_c *other) {
    if (mIsDying) {
        return true;
    }

    daPlBase_c *player = (daPlBase_c *) other->mpOwner;
    mVec3_c pos = mPos;
    pos.x += mCenterOffs.x;
    pos.y += mCenterOffs.y;

    player->slideComboSE(player->mComboMultiplier, false);
    pos = pos; // @reconstruction: forces the (otherwise dead) aggregate to keep an address

    player->mComboMultiplier++;
    if (player->mComboMultiplier >= 8) {
        player->mComboMultiplier = 8;
    }

    int score = mCombo.getComboScore(player->mComboMultiplier);

    u32 plrNo = player->getPlrNo();
    if (plrNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, score, plrNo);
    }

    u8 dir = !(player->mSpeed.x >= 0.0f);
    dActorMng_c::m_instance->createUpCoin(getCenterPos(), dir, 1, 0);

    kill(player);
    return true;
}

bool daEnDpakkunBase_c::hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other) {
    daPlBase_c *player = (daPlBase_c *) other->mpOwner;

    if (mNoHitPlayer.mTimer[player->getPlrNo()] != 0) {
        return true;
    }
    mNoHitPlayer.mTimer[player->getPlrNo()] = 16;

    float speedF;
    if (player->mPos.x > mPos.x) {
        speedF = 1.25f;
    } else {
        speedF = -1.25f;
    }
    player->setJump(4.5f, speedF, true, 0, 0);

    dAudio::g_pSndObjEmy->startSound(SE_EMY_YOSHI_STEP, mPos, 0);
    return true;
}

bool daEnDpakkunBase_c::hitCallback_Star(dCc_c *self, dCc_c *other) {
    if (mIsDying) {
        return true;
    }

    daPlBase_c *player = (daPlBase_c *) other->mpOwner;
    mVec3_c pos = mPos;
    pos.x += mCenterOffs.x;
    pos.y += mCenterOffs.y;
    pos = pos; // @reconstruction: forces the (otherwise dead) aggregate to keep an address

    player->slideComboSE(player->getStarCount(), false);

    int score = dEnCombo_c::calcPlStarCnt(player);

    u32 plrNo = player->getPlrNo();
    if (plrNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, score, plrNo);
    }

    mVec2_c hitPos(self->getCollPosX(), self->getCollPosY());
    hitdamageEffect(mVec3_c(hitPos.x, hitPos.y, 5500.0f));

    kill(player);
    return true;
}

bool daEnDpakkunBase_c::hitCallback_Shell(dCc_c *self, dCc_c *other) {
    if (mIsDying) {
        return true;
    }

    mVec2_c hitPos(self->getCollPosX(), self->getCollPosY());
    dActor_c *actor = other->mpOwner;
    hitdamageEffect(mVec3_c(hitPos.x, hitPos.y, 5500.0f));

    u32 plrNo = actor->getPlrNo();
    if (plrNo <= 3) {
        actor->slideComboSE(actor->mComboMultiplier, false);

        actor->mComboMultiplier++;
        if (actor->mComboMultiplier >= 8) {
            actor->mComboMultiplier = 8;
        }

        dScoreMng_c::m_instance->ScoreSet(this, actor->mComboMultiplier, plrNo);
    } else {
        dAudio::g_pSndObjEmy->startSound(SE_EMY_DOWN, mPos, 0);
    }

    kill(actor);
    return true;
}

void daEnDpakkunBase_c::YoshiFumiJumpSet(dActor_c *actor) {
    PlayerFumiJump(actor, 0.2815f + dAcPy_c::msc_JUMP_SPEED);
}

bool daEnDpakkunBase_c::isPlayerDemo() {
    for (int i = 0; i < PLAYER_COUNT; i++) {
        daPlBase_c *player = daPyMng_c::getPlayer(i);
        if (player != nullptr && player->isDemo()) {
            return true;
        }
    }
    return false;
}

void daEnDpakkunBase_c::kill(dActor_c *killedBy) {
    mIsDying = 1;

    u32 plrNo = killedBy->getPlrNo();
    if (plrNo <= 3) {
        dMultiMng_c::mspInstance->incEnemyDown(plrNo);
    }

    dAudio::g_pSndObjEmy->startSound(SE_EMY_DOKAN_PAKKUN_DOWN, mPos, 0);

    mVec3_c pos = getCenterPos();
    mEf::createEffect("Wm_en_burst_m", 0, &pos, nullptr, nullptr);

    setDeathInfo_Vanish(killedBy);
}

void daEnDpakkunBase_c::setIceAnm() {
    if (mStateMgr.getMainStateID()->isEqual(StateID_Attack)) {
        mAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_freeze"), m3d::FORWARD_ONCE);
        mModel.setAnm(mAnm, 0.0f);
        mAnm.setFrame(0.0f);
    }

    for (int i = 0; i < 9; i++) {
        mNeckAngle[i] = 0;
    }
}

bool daEnDpakkunBase_c::checkQuakeDeath() {
    static const mVec2_c cs_check_ofs[4] = {
        mVec2_c(12.0f, 20.0f),
        mVec2_c(12.0f, 20.0f),
        mVec2_c(20.0f, 12.0f),
        mVec2_c(20.0f, 12.0f)
    };

    mVec3_c center = getCenterPos();

    mVec2_c check[4];
    check[0].x = center.x + cs_check_ofs[mPakkunDir].x; check[0].y = center.y + cs_check_ofs[mPakkunDir].y;
    check[1].x = center.x + cs_check_ofs[mPakkunDir].x; check[1].y = center.y - cs_check_ofs[mPakkunDir].y;
    check[2].x = center.x - cs_check_ofs[mPakkunDir].x; check[2].y = center.y + cs_check_ofs[mPakkunDir].y;
    check[3].x = center.x - cs_check_ofs[mPakkunDir].x; check[3].y = center.y - cs_check_ofs[mPakkunDir].y;

    float startX = dBgParameter_c::ms_Instance_p->xStart();
    float endX = dBgParameter_c::ms_Instance_p->xEnd();
    float startY = dBgParameter_c::ms_Instance_p->yStart();
    float endY = dBgParameter_c::ms_Instance_p->yEnd();

    for (int i = 0; i < 4; i++) {
        if (check[i].x >= startX && check[i].x < endX && check[i].y <= startY && check[i].y > endY) {
            return true;
        }
    }
    return false;
}

void daEnDpakkunBase_c::setDeathInfo_Vanish(dActor_c *killedBy) {
    mIsDying = 1;
    mDeathInfo = (sDeathInfoData) {
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        &StateID_DieVanish,
        -1,
        -1,
        mDirection,
        -1
    };
}

void daEnDpakkunBase_c::setDeathInfo_IceBreak() {
    killIce();

    mIsDying = 1;
    const u8 dir = mIceDeathDirection;

    mDeathInfo = (sDeathInfoData) {
        l_base_fall_speed_x[dir],
        smc_DEADFALL_YSPEED,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        &StateID_DieIceBreak,
        -1,
        -1,
        dir,
        mPlayerNo
    };
}

void daEnDpakkunBase_c::setDeathInfo_Quake(int type) {
    mVec3_c pos;
    pos.x = mStartPos.x + l_hole_offset[mPakkunDir].x;
    pos.y = mStartPos.y + l_hole_offset[mPakkunDir].y;
    pos.z = 5500.0f;

    if (type == 0) {
        EffectManager_c::SetVsHitEffect(&pos);
    } else if (type == 1) {
        EffectManager_c::SetVsHitEffect(&pos);
    }

    dEnemyMng_c::m_instance->breakdownSE(dEnemyMng_c::m_instance->m_154, mPos);
    dEnemyMng_c::m_instance->incQuakeComboCount(0);

    int score = mCombo.getQuakeScore(dEnemyMng_c::m_instance->m_154);
    if (score >= 0) {
        dScoreMng_c::m_instance->UnKnownScoreSet(this, score, 0.0f, 24.0f);
    }

    mIsDying = 1;

    u8 dir = dScStage_c::m_exeFrame & 1;
    if (dEnemyMng_c::m_instance->m_154 & 1) {
        dir ^= 1;
    }

    mDeathInfo = (sDeathInfoData) {
        l_base_fall_speed_x[dir],
        smc_DEADFALL_YSPEED,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        &StateID_DieIceBreak,
        mCombo.getDamageScore(),
        -1,
        dir,
        -1
    };
}

void daEnDpakkunBase_c::setIceBreakAnm() {
    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dead_ice"), m3d::FORWARD_LOOP);
    mModel.setAnm(mAnm, 0.0f);
}

void daEnDpakkunBase_c::initializeState_DieVanish() {
    setVanishAnm();
    removeCc();
    mBc.mFlags = 0;
    mAngle.y = 0;
}

void daEnDpakkunBase_c::setVanishAnm() {}

void daEnDpakkunBase_c::finalizeState_DieVanish() {}

void daEnDpakkunBase_c::executeState_DieVanish() {
    mModel.play();

    if (mAnm.isStop()) {
        deleteActor(1);
    }
}

void daEnDpakkunBase_c::initializeState_DieIceBreak() {
    setIceBreakAnm();
    removeCc();
    mBc.mFlags = 0;

    mSpeed.set(mDeathInfo.getXSpeed(), mDeathInfo.getYSpeed(), 0.0f);

    mSpeedMax.x = 0.0f;
    mSpeedMax.y = mDeathInfo.getMaxYSpeed();
    mSpeedMax.z = 0.0f;

    mIceDeathDirection = mDeathInfo.mDirection;

    mAccelF = 0.0f;
    mAccelY = mDeathInfo.getYAccel();

    mScale.x = 0.8f;
    mScale.y = 0.8f;
    mScale.z = 0.8f;

    u32 plrNo = mDeathInfo.mKilledBy;
    if (mDeathInfo.mScore >= 0 && plrNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, mDeathInfo.mScore, plrNo);
    }

    mAngle.y = 0x4000;

    if (mAmiLayer == 1) {
        mPos.z = -384.0f;
    } else {
        mPos.z = 4500.0f;
    }
}

void daEnDpakkunBase_c::finalizeState_DieIceBreak() {}

void daEnDpakkunBase_c::executeState_DieIceBreak() {
    static const s16 cs_spin_speed[2] = { 0x100, -0x100 };
    static const s16 cs_fall_spin_speed[2] = { -0xC00, 0xC00 };

    mModel.play();
    calcSpeedY();
    posMove();

    mSpinAngle += cs_fall_spin_speed[mIceDeathDirection];

    if (mDirection == mIceDeathDirection) {
        mAngle.y -= cs_spin_speed[mIceDeathDirection];
    } else {
        mAngle.y += cs_spin_speed[mIceDeathDirection];
    }

    WaterCheck(mPos, 1.0f);
}

void daEnDpakkunBase_c::initializeState_Wait() {}
void daEnDpakkunBase_c::finalizeState_Wait() {}
void daEnDpakkunBase_c::executeState_Wait() {}

void daEnDpakkunBase_c::initializeState_Appear() {}
void daEnDpakkunBase_c::finalizeState_Appear() {}
void daEnDpakkunBase_c::executeState_Appear() {}

void daEnDpakkunBase_c::initializeState_Attack() {}
void daEnDpakkunBase_c::finalizeState_Attack() {}
void daEnDpakkunBase_c::executeState_Attack() {}

void daEnDpakkunBase_c::initializeState_Disappear() {}
void daEnDpakkunBase_c::finalizeState_Disappear() {}
void daEnDpakkunBase_c::executeState_Disappear() {}

void daEnDpakkunBase_c::nodeCallback_c::timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl) {
    float deg = mpOwner->mNeckAngle[nodeId] * 90 / 0x4000;

    nw4r::math::VEC3 rot;
    anmRes->GetRotateDeg(&rot);
    rot.x += deg;
    anmRes->SetRotateDeg(&rot);
}
