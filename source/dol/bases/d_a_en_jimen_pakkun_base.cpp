#include <game/bases/d_a_en_jimen_pakkun_base.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_effectmanager.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_score_manager.hpp>
#include <game/mLib/m_heap.hpp>
#include <constants/sound_list.h>

/// @brief The foot sensor. @unofficial
extern const sBcSensorPoint l_jimen_pakkun_foot;

/// @brief The collider template shared by all three of this actor's colliders. @unofficial
extern const sCcDatNewF l_pakkun_cc;

const sBcSensorPoint l_jimen_pakkun_foot = { 0x400, 0, 0 };

const sCcDatNewF l_pakkun_cc = {
    0.0f, 10.0f,
    4.0f, 10.0f,
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) |
        BIT_FLAG(CC_KIND_YOSHI) | BIT_FLAG(CC_KIND_ENEMY) |
        BIT_FLAG(CC_KIND_TAMA),
    (u32) ~(BIT_FLAG(CC_ATTACK_NONE) | BIT_FLAG(CC_ATTACK_YOSHI_MOUTH) |
        BIT_FLAG(CC_ATTACK_SPIN_LIFT_UP) | BIT_FLAG(CC_ATTACK_SAND_PILLAR)),
    0,
    dEn_c::normal_collcheck
};

STATE_VIRTUAL_DEFINE(daEnJimenPakkunBase_c, DieOther);
STATE_VIRTUAL_DEFINE(daEnJimenPakkunBase_c, DieIceBreak);
STATE_VIRTUAL_DEFINE(daEnJimenPakkunBase_c, Attack);
STATE_VIRTUAL_DEFINE(daEnJimenPakkunBase_c, SakasaAttack);

// 0x8002EF80
int daEnJimenPakkunBase_c::create() {
    createMdl();

    mCc.set(this, (sCcDatNewF *) &l_pakkun_cc);
    mHeadCc.set(this, (sCcDatNewF *) &l_pakkun_cc);
    mBodyCc.set(this, (sCcDatNewF *) &l_pakkun_cc);

    mSakasa = ACTOR_PARAM(Sakasa);
    mWaterCheckHeight = 1.0f;

    initialize();

    mCc.entry();
    mHeadCc.entry();
    mBodyCc.entry();

    if (mSakasa == 1) {
        mHeadCc.mCcData.mBase.mOffset.y = -mBodyCc.mCcData.mBase.mOffset.y;
        mCenterOffs.y = -mCenterOffs.y;
    }

    mActorProperties |= BIT_FLAG(9);

    mBc.set(this, l_jimen_pakkun_foot, nullptr, nullptr);

    mHeadPos = mPos;
    mIsDying = 0;

    calcMdl();
    calcCcInfo();

    initAction();
    entryHIO();

    return SUCCEEDED;
}

// 0x8002F0D0 entryHIO -- inline in the header, flushes here.

// 0x8002F0E0
void daEnJimenPakkunBase_c::createMdl() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("pakkun", "g3d/pakkun.brres");

    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR, 1);
    dActor_c::setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr chrAnm = mResFile.GetResAnmChr("attack");
    mAnm.create(mdl, chrAnm, &mAllocator);

    nw4r::g3d::ResAnmClr clrAnm = mResFile.GetResAnmClr("damage");
    mClrAnm.create(mdl, clrAnm, &mAllocator);
    mClrAnm.setAnm(mModel, clrAnm, 0, m3d::FORWARD_ONCE);
    mModel.setAnm(mClrAnm);
    mClrAnm.setFrame(0.0f, 0);
    mClrAnm.setRate(0.0f, 0);

    mAllocator.adjustFrmHeap();
}

// 0x8002F250
int daEnJimenPakkunBase_c::execute() {
    mStateMgr.executeState();

    if (mNoRespawn) {
        mClrAnm.setFrame(0.0f, 0);
        mClrAnm.setRate(0.0f, 0);
    }

    mClrAnm.play();

    WaterCheck(mPos, mWaterCheckHeight);
    ActorScrOutCheck(0);
    return SUCCEEDED;
}

// 0x8002F2F0
void daEnJimenPakkunBase_c::postExecute(fBase_c::MAIN_STATE_e status) {
    if (status == SUCCESS) {
        mBodyCc.clear();
        mHeadCc.clear();
    }
    dEn_c::postExecute(status);
}

// 0x8002F350
int daEnJimenPakkunBase_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// 0x8002F380
void daEnJimenPakkunBase_c::deleteReady() {}

// 0x8002F390
int daEnJimenPakkunBase_c::doDelete() {
    mHeadCc.release();
    mBodyCc.release();
    removeHIO();
    return SUCCEEDED;
}

// 0x8002F3E0 removeHIO -- inline in the header, flushes here.

// 0x8002F3F0
void daEnJimenPakkunBase_c::calcMdl() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    pos.y -= 1.0f;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.ZrotM(angle.z);
    mMatrix.YrotM(angle.y);
    mMatrix.concat(mMtx_c::createTrans(0.0f, 16.0f, 0.0f));
    mMatrix.XrotM(angle.x);
    mMatrix.concat(mMtx_c::createTrans(0.0f, -16.0f, 0.0f));

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mBoyoMng.getScale());
    mModel.calc(false);

    calcJnt();
}

// 0x8002F540
void daEnJimenPakkunBase_c::calcCcInfo() {
    mCc.mCcData.mBase.mOffset.x = mHeadPos.x - mPos.x;
    mCc.mCcData.mBase.mOffset.y = mHeadPos.y - mPos.y;

    float ofs = mSakasa == 1 ? -28.0f : 28.0f;

    float offsX = (mStemPos.x + mPos.x) * 0.5f - mPos.x;
    float offsY = (ofs + (mStemPos.y + mPos.y)) * 0.5f - mPos.y;
    float sizeX = std::fabs((mStemPos.x - mPos.x) * 0.5f);
    float sizeY = std::fabs((mStemPos.y - mPos.y) * 0.5f);

    if (sizeX < 3.0f) {
        sizeX = 3.0f;
    }

    mBodyCc.mCcData.mBase.mOffset.set(offsX, offsY);
    mBodyCc.mCcData.mBase.mSize.set(sizeX, sizeY);
}

// 0x8002F5F0
void daEnJimenPakkunBase_c::initialize() {
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

    mCenterOffs.x = 0.0f;
    mCenterOffs.y = 16.0f;
    mCenterOffs.z = 0.0f;

    mCc.mCcData.mBase.mSize.x = 8.0f;
    mCc.mCcData.mBase.mSize.y = 8.0f;

    mVisibleAreaOffset.x = 0.0f;
    mVisibleAreaOffset.y = 16.0f;

    mEatBehavior = EAT_TYPE_DRINK;
}

// 0x8002F640
int daEnJimenPakkunBase_c::initAction() {
    if (mSakasa == 1) {
        mPos.y += getSakasaOfs();
        mAngle.z = 0x8000;
        changeState(StateID_SakasaAttack);
    } else {
        changeState(StateID_Attack);
    }

    return SUCCEEDED;
}

// 0x8002F6E0 getSakasaOfs -- inline in the header, flushes here.

// 0x8002F6F0
void daEnJimenPakkunBase_c::returnState_Ice() {
    if (mSakasa == 1) {
        changeState(StateID_SakasaAttack);
    } else {
        changeState(StateID_Attack);
    }
}

// 0x8002F730
void daEnJimenPakkunBase_c::calcJnt() {
    mMtx_c mtx;
    mModel.getNodeWorldMtx(6, &mtx);

    mtx.concat(mMtx_c::createTrans(0.0f, 12.0f, 0.0f));
    mtx.multVecZero(mHeadPos);

    mtx.concat(mMtx_c::createTrans(0.0f, -12.0f, 0.0f));
    mtx.concat(mMtx_c::createTrans(0.0f, 8.0f, 0.0f));
    mtx.multVecZero(mStemPos);

    mtx.concat(mMtx_c::createTrans(0.0f, -8.0f, 0.0f));
    mtx.concat(mMtx_c::createTrans(0.0f, 12.0f, 0.0f));
    mtx.multVecZero(mNeckPos);

    mtx.zero();
    mModel.getNodeWorldMtx(8, &mtx);
    mtx.multVecZero(mMouthPos);
}

// 0x8002F870
void daEnJimenPakkunBase_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *player = other->mpOwner;
    if (!LineBoundaryCheck(player)) {
        setDamage(player);
        if (other->mCcData.mAttack == CC_ATTACK_PENGUIN_SLIDE) {
            mNoHitPlayer.mTimer[player->getPlrNo()] = 0x1C;
        }
    }
}

// 0x8002F910
void daEnJimenPakkunBase_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    if (!Enfumi_check(self, other, 1)) {
        dEn_c::Normal_VsYoshiHitCheck(self, other);
    }
}

// 0x8002F970
bool daEnJimenPakkunBase_c::hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other) {
    dActor_c *player = other->mpOwner;

    if (mNoHitPlayer.mTimer[player->getPlrNo()] != 0) {
        return true;
    }
    mNoHitPlayer.mTimer[player->getPlrNo()] = 16;

    YoshiFumiJumpSet(player);
    dAudio::g_pSndObjEmy->startSound(SE_EMY_YOSHI_STEP, mPos, 0);
    return true;
}

// 0x8002FA50
bool daEnJimenPakkunBase_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    if (mIsDying) {
        return true;
    }
    mIsDying = 1;
    boyonBegin();

    dActor_c *player = other->mpOwner;
    u8 dir = !(player->mSpeed.x >= 0.0f);
    dActorMng_c::m_instance->createUpCoin(getCenterPos(), dir, 1, 0);

    dScoreMng_c *scoreMng;
    u32 plrNo = player->getPlrNo();
    if (plrNo <= 3) {
        scoreMng = dScoreMng_c::m_instance;
        scoreMng->ScoreSet(this, mCombo.getDamageScore(), plrNo);
    }

    dAudio::g_pSndObjEmy->startSound(SE_EMY_DOWN, mPos, 0);
    setDeathInfo_Other(player);
    return true;
}

// 0x8002FBA0
bool daEnJimenPakkunBase_c::hitCallback_Ice(dCc_c *self, dCc_c *other) {
    changeState(StateID_Ice);
    return true;
}

// 0x8002FBE0
bool daEnJimenPakkunBase_c::hitCallback_Shell(dCc_c *self, dCc_c *other) {
    if (mIsDying) {
        return true;
    }
    mIsDying = 1;

    mVec2_c hitPos(self->getCollPosX(), self->getCollPosY());
    hitdamageEffect(mVec3_c(hitPos.x, hitPos.y, 5500.0f));

    dActor_c *actor = other->mpOwner;
    u32 plrNo = actor->getPlrNo();
    if (plrNo <= 3) {
        actor->slideComboSE(actor->mComboMultiplier, false);

        actor->mComboMultiplier++;
        if (actor->mComboMultiplier >= 8) {
            actor->mComboMultiplier = 8;
        }

        int score = mCombo.getComboScore(actor->mComboMultiplier);
        dScoreMng_c *scoreMng = dScoreMng_c::m_instance;
        scoreMng->ScoreSet(this, score, actor->getPlrNo());
    } else {
        dAudio::g_pSndObjEmy->startSound(SE_EMY_DOWN, mPos, 0);
    }

    setDeathInfo_Other(actor);
    return true;
}

// 0x8002FD50
bool daEnJimenPakkunBase_c::hitCallback_Star(dCc_c *self, dCc_c *other) {
    if (mIsDying) {
        return true;
    }
    mIsDying = 1;

    mVec2_c hitPos(self->getCollPosX(), self->getCollPosY());
    hitdamageEffect(mVec3_c(hitPos.x, hitPos.y, 5500.0f));

    daPlBase_c *player = (daPlBase_c *) other->mpOwner;
    player->slideComboSE(player->getStarCount(), false);

    int score = dEnCombo_c::calcPlStarCnt(player);

    u32 plrNo = player->getPlrNo();
    if (plrNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, score, plrNo);
    }

    setDeathInfo_Other(player);
    return true;
}

// 0x8002FE60
bool daEnJimenPakkunBase_c::hitCallback_YoshiFire(dCc_c *self, dCc_c *other) {
    if (mIsDying) {
        return true;
    }
    mIsDying = 1;

    mVec2_c hitPos(self->getCollPosX(), self->getCollPosY());
    hitdamageEffect(mVec3_c(hitPos.x, hitPos.y, 5500.0f));

    dActor_c *actor = other->mpOwner;
    actor->slideComboSE(actor->mComboMultiplier, false);

    actor->mComboMultiplier++;
    if (actor->mComboMultiplier >= 8) {
        actor->mComboMultiplier = 8;
    }

    int score = mCombo.getComboScore(actor->mComboMultiplier);

    u32 plrNo = actor->getPlrNo();
    if (plrNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, score, plrNo);
    }

    setDeathInfo_Other(actor);
    return true;
}

// 0x8002FF80
bool daEnJimenPakkunBase_c::ActorDrawCullCheck() {
    sRangeDataF bound;
    bound.mOffset.x = mVisibleAreaOffset.x;
    bound.mOffset.y = mVisibleAreaOffset.y;
    bound.mSize.x = 0.5f * mVisibleAreaSize.x;
    bound.mSize.y = 0.5f * mVisibleAreaSize.y;

    mIsOffscreen = false;
    if (dGameCom::someCheck(&mPos, &bound)) {
        mIsOffscreen = true;
    }
    return false;
}

// 0x80030000
void daEnJimenPakkunBase_c::YoshiFumiJumpSet(dActor_c *actor) {
    PlayerFumiJump(actor, 4.0f);
}

// 0x80030010
bool daEnJimenPakkunBase_c::createIceActor() {
    mVec3_c offset(mPos.x, mPos.y - 2.5f, mPos.z);

    if (mSakasa == 1) {
        offset.y = mPos.y - 44.0f;
    }

    int mode = 1;

    dIceInfo iceInfo[] = {
        {
            mode,
            offset,
            mVec3_c(1.55f, 1.2f, 2.0f)
        }
    };
    return mIceMng.createIce(iceInfo, ARRAY_SIZE(iceInfo));
}

// 0x80030140
void daEnJimenPakkunBase_c::setAnm(char *name, m3d::playMode_e playMode, float blendFrame) {
    mAnm.setAnm(mModel, mResFile.GetResAnmChr(name), playMode);
    mModel.setAnm(mAnm, blendFrame);
}

// 0x800301B0
void daEnJimenPakkunBase_c::setIceAnm() {
    setAnm("freeze", m3d::FORWARD_ONCE, 0.0f);
    mAnm.setRate(0.0f);

    if (mAngle.y >= 0) {
        mDirection = 0;
    } else {
        mDirection = 1;
    }

    mAngle.y = l_base_angleY[mDirection];
}

// 0x80030230
void daEnJimenPakkunBase_c::setDeathInfo_IceBreak() {
    killIce();

    u8 dir = mIceDeathDirection;
    u8 plrNo = mPlayerNo;

    mDeathInfo = (sDeathInfoData) {
        l_base_fall_speed_x[dir],
        smc_DEADFALL_YSPEED,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        &StateID_DieIceBreak,
        -1,
        -1,
        dir,
        plrNo
    };
}

// 0x80030310
void daEnJimenPakkunBase_c::setDeathInfo_Quake(int type) {
    mVec3_c center(getCenterX(), getCenterY(), 5500.0f);

    if (type == 0) {
        EffectManager_c::SetVsHitEffect(&center);
    } else if (type == 1) {
        EffectManager_c::SetVsHitEffect(&center);
    }

    dEnemyMng_c::m_instance->breakdownSE(dEnemyMng_c::m_instance->m_154, mPos);
    dEnemyMng_c::m_instance->incQuakeComboCount(0);

    int comboScore = mCombo.getQuakeScore(dEnemyMng_c::m_instance->m_154);

    mDeathInfo = (sDeathInfoData) {
        0.0f,
        0.0f,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        &dEn_c::StateID_DieOther,
        comboScore,
        -1,
        mDirection,
        dDeathInfo_c::smc_UNKNOWN_HIT
    };
}

// 0x80030440
void daEnJimenPakkunBase_c::initializeState_DieOther() {
    setAnm("dead", m3d::FORWARD_ONCE, 1.0f);
    removeCc();
    downSE();

    int score = mDeathInfo.mScore;
    int plrNo = mDeathInfo.mKilledBy;
    if (score >= 0) {
        if (plrNo >= 0 && plrNo < PLAYER_COUNT) {
            dScoreMng_c::m_instance->ScoreSet(this, score, plrNo);
        } else if (plrNo == dDeathInfo_c::smc_UNKNOWN_HIT) {
            dScoreMng_c::m_instance->UnKnownScoreSet(this, score, 0.0f, 24.0f);
        }
    }

    m_23b = 1;
}

// 0x80030500 removeCc -- inline in the header, flushes here.

// 0x80030540
void daEnJimenPakkunBase_c::downSE() {
    dAudio::g_pSndObjEmy->startSound(SE_EMY_PAKKUN_DOWN, mPos, 0);
}

// 0x80030590
void daEnJimenPakkunBase_c::finalizeState_DieOther() {}

// 0x800305A0
void daEnJimenPakkunBase_c::executeState_DieOther() {
    mModel.play();

    if (mSakasa == 0) {
        calcSpeedY();
        posMove();
        if (EnBgCheckFoot()) {
            mSpeed.y = 0.0f;
        }
    }

    switch (m_23b) {
        case 1: {
            if (mAnm.isStop()) {
                setAnm("dead_leaf", m3d::FORWARD_ONCE, 1.0f);
                mAngle.y = 0;
                m_23b = 2;
            }
            break;
        }

        case 2: {
            if (mAnm.isStop()) {
                deleteActor(1);
            }
            break;
        }
    }
}

// 0x80030680
void daEnJimenPakkunBase_c::initializeState_DieIceBreak() {
    setAnm("dead_ice", m3d::FORWARD_LOOP, 0.0f);
    removeCc();

    mBc.mFlags = 0;

    mSpeed.set(mDeathInfo.getXSpeed(), mDeathInfo.getYSpeed(), 0.0f);

    mSpeedMax.x = 0.0f;
    mSpeedMax.y = mDeathInfo.getMaxYSpeed();
    mSpeedMax.z = 0.0f;

    mIceDeathDirection = mDeathInfo.mDirection;

    if (mAngle.y >= 0) {
        mDirection = 0;
    } else {
        mDirection = 1;
    }

    mAngle.y = l_base_angleY[mDirection];

    mAccelF = 0.0f;
    mAccelY = mDeathInfo.getYAccel();

    u32 plrNo = mDeathInfo.mKilledBy;
    if (mDeathInfo.mScore >= 0 && plrNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, mDeathInfo.mScore, plrNo);
    }

    if (mAmiLayer == 1) {
        mPos.z = -384.0f;
    } else {
        mPos.z = 4500.0f;
    }
}

// 0x800307A0
void daEnJimenPakkunBase_c::finalizeState_DieIceBreak() {}

// 0x800307B0
void daEnJimenPakkunBase_c::executeState_DieIceBreak() {
    static const s16 cs_spin_speed[2] = { 0x100, -0x100 };

    mModel.play();
    calcSpeedY();
    posMove();

    if (mDirection == mIceDeathDirection) {
        mAngle.x += 0xC00;
        mAngle.y -= cs_spin_speed[mIceDeathDirection];
    } else {
        mAngle.x -= 0xC00;
        mAngle.y += cs_spin_speed[mIceDeathDirection];
    }

    WaterCheck(mPos, 1.0f);
}

// 0x80030870
void daEnJimenPakkunBase_c::initializeState_Attack() {}
// 0x80030880
void daEnJimenPakkunBase_c::finalizeState_Attack() {}
// 0x80030890
void daEnJimenPakkunBase_c::executeState_Attack() {}

// 0x800308A0
void daEnJimenPakkunBase_c::initializeState_SakasaAttack() {}
// 0x800308B0
void daEnJimenPakkunBase_c::finalizeState_SakasaAttack() {}
// 0x800308C0
void daEnJimenPakkunBase_c::executeState_SakasaAttack() {}

// 0x800308D0
void daEnJimenPakkunBase_c::reviveCc() {
    dActor_c::mCc.entry();
    mHeadCc.entry();
    mBodyCc.entry();
}

// 0x80030910
void daEnJimenPakkunBase_c::finalUpdate() {
    calcMdl();
    calcCcInfo();
}

// 0x80030950
BOOL daEnJimenPakkunBase_c::isQuakeDamage() {
    return !isState(dEn_c::StateID_Ice);
}

// 0x800309B0 YoshiFumiScoreSet, 0x800309C0 block_hit_init, 0x800309D0 hitCallback_Slip,
// 0x800309E0 hitCallback_HipAttk, 0x800309F0 hitCallback_Spin, 0x80030A00 __dt --
// all inline in the header, emitted by the end-of-TU flush block.
