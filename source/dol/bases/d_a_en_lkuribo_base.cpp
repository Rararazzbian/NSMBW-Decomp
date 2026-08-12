#include <game/bases/d_a_en_lkuribo_base.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_score_manager.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/sLib/s_lib.hpp>
#include <constants/sound_list.h>

/// @note Byte-identical to daEnKuriboBase_c's collider of the same name.
static const sCcDatNewF l_kuribo_cc = {
    0.0f, 8.0f,
    8.0f, 8.0f,
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) |
        BIT_FLAG(CC_KIND_YOSHI) | BIT_FLAG(CC_KIND_ENEMY) |
        BIT_FLAG(CC_KIND_ITEM) | BIT_FLAG(CC_KIND_TAMA),
    (u32) ~(BIT_FLAG(CC_ATTACK_NONE) | BIT_FLAG(CC_ATTACK_YOSHI_MOUTH) | BIT_FLAG(CC_ATTACK_SPIN_LIFT_UP) | BIT_FLAG(CC_ATTACK_SAND_PILLAR)),
    CC_STATUS_NONE,
    dEn_c::normal_collcheck
};

STATE_VIRTUAL_DEFINE(daEnLkuriboBase_c, Walk);
STATE_VIRTUAL_DEFINE(daEnLkuriboBase_c, Turn);
STATE_VIRTUAL_DEFINE(daEnLkuriboBase_c, Press);
STATE_VIRTUAL_DEFINE(daEnLkuriboBase_c, Split);
STATE_VIRTUAL_DEFINE(daEnLkuriboBase_c, HipSplit);
STATE_VIRTUAL_DEFINE(daEnLkuriboBase_c, DieFall);

int daEnLkuriboBase_c::create() {
    createMdl();

    mSpeedMax.y = -4.0f;

    mDirection = getPl_LRflag(mPos);
    mAngle.y = l_base_angleY[mDirection];
    mFumiProc.mFumiCheck.m_00 = 2;

    mCc.set(this, (sCcDatNewF *) &l_kuribo_cc);
    mCc.entry();

    mActorProperties |= 0x200;

    setBoyoFunc(&daEnLkuriboBase_c::nonBoyoProc);
    mEatBehavior = 0;

    initialize();

    return SUCCEEDED;
}

void daEnLkuriboBase_c::initialize() {}

void daEnLkuriboBase_c::createMdl() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("kuriboBig", "g3d/kuriboBig.brres");

    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("kuriboBig");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::ANM_TEXPAT, 1);
    dActor_c::setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr("walk");
    mAnmChr.create(mdl, anm, &mAllocator);

    mResAnmTexPat = mResFile.GetResAnmTexPat("walk");
    mAnmTexPat.create(mdl, mResAnmTexPat, &mAllocator);
    mAnmTexPat.setAnm(mModel, mResAnmTexPat, 0, m3d::FORWARD_LOOP);

    mAllocator.adjustFrmHeap();
}

int daEnLkuriboBase_c::doDelete() {
    return SUCCEEDED;
}

int daEnLkuriboBase_c::execute() {
    mStateMgr.executeState();

    if (HasamareBgCheck()) {
        setDeathInfo_Hasami();
    } else {
        ActorScrOutCheck(0);
    }

    return SUCCEEDED;
}

int daEnLkuriboBase_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

void daEnLkuriboBase_c::calcMdl() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.YrotM(angle.y);
    mMatrix.concat(mMtx_c::createTrans(0.0f, mCenterOffs.y, 0.0f));
    mMatrix.XrotM(angle.x);
    mMatrix.concat(mMtx_c::createTrans(0.0f, -mCenterOffs.y, 0.0f));
    mMatrix.ZrotM(angle.z);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale + mBoyoScale);
    mModel.calc(false);
}

void daEnLkuriboBase_c::calcJnt() {
    nw4r::g3d::ResMdl resMdl = mModel.getResMdl();

    nw4r::g3d::ResNode legLeft = resMdl.GetResNode("leg_left");
    mMtx_c matrix;
    mModel.getNodeWorldMtx(legLeft.GetID(), &matrix);
    matrix.multVecZero(mLegLeftPos);

    nw4r::g3d::ResNode legRight = resMdl.GetResNode("leg_right");
    mModel.getNodeWorldMtx(legRight.GetID(), &matrix);
    matrix.multVecZero(mLegRightPos);

    nw4r::g3d::ResNode brow = resMdl.GetResNode("brow");
    mModel.getNodeWorldMtx(brow.GetID(), &matrix);
    matrix.multVecZero(mBrowPos);
}

void daEnLkuriboBase_c::Normal_VsEnHitCheck(dCc_c *self, dCc_c *other) {
    float xOfs = self->getXOffset(CC_KIND_ENEMY);
    if ((mDirection == 1 && xOfs > 0.0f) || (mDirection == 0 && xOfs < 0.0f)) {
        if (!isState(StateID_Turn)) {
            changeState(StateID_Turn);
        }
    }
}

void daEnLkuriboBase_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    switch (Enfumi_check(self, other, 0)) {
        case 1:
            splitSE();
            splitEffect();
            self->mInfo |= CC_NO_HIT;
            changeState(StateID_Press);
            break;

        case 3:
            splitSE();
            splitEffect();
            self->mInfo |= CC_NO_HIT;
            changeState(StateID_Press);
            break;

        case 0:
            dEn_c::Normal_VsPlHitCheck(self, other);
            break;
    }
}

void daEnLkuriboBase_c::splitSE() {
    dAudio::g_pSndObjEmy->startSound(SE_EMY_KURIBO_M_SPLIT, mPos, 0);
}

void daEnLkuriboBase_c::splitEffect() {}

void daEnLkuriboBase_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    switch (Enfumi_check(self, other, 2)) {
        case 1:
            splitSE();
            splitEffect();
            self->mInfo |= CC_NO_HIT;
            changeState(StateID_Press);
            break;

        case 0:
            dEn_c::Normal_VsYoshiHitCheck(self, other);
            break;
    }
}

bool daEnLkuriboBase_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    dActor_c *fire = other->getOwner();
    u8 dir = !(fire->mSpeed.x >= 0.0f);

    if (--mFireHp > 0) {
        setBoyoFunc(&daEnLkuriboBase_c::fireBoyoProc);
        firehitSE();
    } else {
        u8 plrNo = fire->getPlrNo();
        setDeathSound_Fire();
        dActorMng_c::m_instance->createUpCoin(getCenterPos(), dir, 3, 0);
        mDeathInfo = (sDeathInfoData) {
            l_base_fall_speed_x[dir],
            smc_DEADFALL_YSPEED,
            smc_DEADFALL_YSPEED_MAX,
            smc_DEADFALL_GRAVITY,
            // NOTE: must be qualified. This class declares its own StateID_DieFall,
            // which shadows dEn_c's, and the target relocates this word to
            // StateID_DieFall__5dEn_c rather than StateID_DieFall__17daEnLkuriboBase_c.
            &dEn_c::StateID_DieFall,
            mCombo.getDamageScore(),
            -1,
            dir,
            plrNo
        };
    }
    return true;
}

void daEnLkuriboBase_c::firehitSE() {
    dAudio::g_pSndObjEmy->startSound(SE_EMY_KURIBO_M_DAMAGE, mPos, 0);
}

bool daEnLkuriboBase_c::hitCallback_HipAttk(dCc_c *self, dCc_c *other) {
    dActor_c *actor = other->getOwner();
    mPressX = actor->mPos.x;
    mPressPlayerNo = actor->getPlrNo();

    dScoreMng_c *scoreMng = dScoreMng_c::m_instance;
    scoreMng->ScoreSet(this, dEnCombo_c::calcPlFumiCnt(actor), actor->getPlrNo());

    hipsplitSE();
    hipsplitEffect();
    changeState(StateID_HipSplit);
    return true;
}

void daEnLkuriboBase_c::hipsplitSE() {
    dAudio::g_pSndObjEmy->startSound(SE_EMY_KURIBO_M_SPLIT, mPos, 0);
}

void daEnLkuriboBase_c::hipsplitEffect() {}

bool daEnLkuriboBase_c::hitCallback_Spin(dCc_c *self, dCc_c *other) {
    dActor_c *actor = other->getOwner();

    dScoreMng_c *scoreMng = dScoreMng_c::m_instance;
    scoreMng->ScoreSet(this, dEnCombo_c::calcPlFumiCnt(actor), actor->getPlrNo());

    FumiJumpSet(actor);
    splitSE();
    splitEffect();
    changeState(StateID_Press);
    return true;
}

bool daEnLkuriboBase_c::setDamage(dActor_c *actor) {
    dAcPy_c *player = (dAcPy_c *) actor;
    if (player->setDamage(this, daPlBase_c::DAMAGE_DEFAULT)) {
        setTurnByPlayerHit(actor);
        return true;
    }
    return false;
}

void daEnLkuriboBase_c::setDeathInfo_Hasami() {
    u8 dir = !(mPos.x - mLastPos.x >= 0.0f);

    mVec2_c pos2D(mPos.x, mPos.y);
    hitdamageEffect(mVec3_c(pos2D, 5500.0f));

    dAudio::g_pSndObjEmy->startSound(SE_EMY_DOWN, mPos, 0);

    mDeathInfo = (sDeathInfoData) {
        l_base_fall_speed_x[dir],
        smc_DEADFALL_YSPEED,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        // NOTE: must be qualified -- see hitCallback_Fire above.
        &dEn_c::StateID_DieFall,
        -1,
        -1,
        dir,
        0xFF
    };
}

void daEnLkuriboBase_c::setBoyoFunc(void (daEnLkuriboBase_c::*func)()) {
    if (mBoyoFunc != nullptr) {
        mBoyoStep = 2;
        (this->*mBoyoFunc)();
    }

    mBoyoFunc = func;
    mBoyoStep = 0;
    (this->*mBoyoFunc)();
}

void daEnLkuriboBase_c::calcBoyonScale() {
    (this->*mBoyoFunc)();
}

void daEnLkuriboBase_c::nonBoyoProc() {
    mBoyoScale.x = 0.0f;
    mBoyoScale.y = 0.0f;
    mBoyoScale.z = 0.0f;
}

void daEnLkuriboBase_c::fireBoyoProc() {
    switch (mBoyoStep) {
        case 0: {
            mBoyoScale.x = 0.0f;
            mBoyoScale.y = 0.0f;
            mBoyoScale.z = 0.0f;
            m_76c = 0;
            m_768 = 2;
            mBoyoStep = 1;
            break;
        }

        case 1: {
            m_76c += 0x2000;

            s16 idx = m_76c;
            idx -= 0x4000;

            float wave = 1.0f + nw4r::math::SinIdx(idx);

            float scale = 0.2f;
            if (m_768 < 2) {
                scale *= 0.5f;
            }

            float boyo = scale * wave * 0.5f;
            mBoyoScale.x = boyo;
            mBoyoScale.y = boyo;
            mBoyoScale.z = boyo;

            if (m_76c == 0) {
                if (--m_768 <= 0) {
                    setBoyoFunc(&daEnLkuriboBase_c::nonBoyoProc);
                }
            }
            break;
        }
    }
}

void daEnLkuriboBase_c::setAnm(char *name, m3d::playMode_e playMode, float rate) {
    mAnmChr.setAnm(mModel, mResFile.GetResAnmChr(name), playMode);
    mModel.setAnm(mAnmChr, rate);
    mAnmChr.setRate(1.0f);
}

void daEnLkuriboBase_c::setTexAnm(char *name, m3d::playMode_e playMode) {
    mAnmTexPat.setAnm(mModel, mResFile.GetResAnmTexPat(name), 0, playMode);
    mModel.setAnm(mAnmTexPat);
    mAnmTexPat.setFrame(0.0f, 0);
    mAnmTexPat.setRate(1.0f, 0);
}

void daEnLkuriboBase_c::setTurnByPlayerHit(dActor_c *actor) {
    mDirection = getTrgToSrcDir_Main(actor->getCenterX(), getCenterX());

    if (isState(StateID_Turn)) {
        changeState(StateID_Walk);
    }

    mAngle.y = l_base_angleY[mDirection];
    setWalkSpeed();
}

void daEnLkuriboBase_c::setWalkSpeed() {}

void daEnLkuriboBase_c::FumiJumpSet(dActor_c *actor) {
    float jumpSpeed = 0.2815f + dAcPy_c::msc_JUMP_SPEED;
    float speedF = actor->mSpeedF;
    ((daPlBase_c *) actor)->setJump(jumpSpeed, speedF, 1, 1, 2);
    dEnemyMng_c::m_instance->m_138 = 1;
}

void daEnLkuriboBase_c::pressAttach() {
    dAcPy_c *player = daPyMng_c::getPlayer(mPressPlayerNo);

    mVec3_c pos;
    pos.x = mPressX;
    pos.y = mBrowPos.y;
    pos.z = mBrowPos.z;

    player->setHipAttackOnEnemy(&pos);
}

void daEnLkuriboBase_c::initializeState_Walk() {
    if (*mStateMgr.getOldStateID() != StateID_Turn) {
        setWalkAnm();
    }

    setWalkSpeed();

    mAccelY = -0.1875f;
    mSpeedMax.set(0.0f, -4.0f, 0.0f);
}

void daEnLkuriboBase_c::setWalkAnm() {}

void daEnLkuriboBase_c::finalizeState_Walk() {}

void daEnLkuriboBase_c::executeState_Walk() {
    mModel.play();
    mAnmTexPat.play();
    calcSpeedY();
    posMove();
    sLib::chaseAngle(&mAngle.y.mAngle, l_base_angleY[mDirection], 0x200);
    walkEffect();

    if (EnBgCheck() & 1) {
        mSpeed.y = 0.0f;
    }

    if (mBc.isWall(mDirection)) {
        mPos.x = mLastPos.x;
        changeState(StateID_Turn);
        mStateMgr.refreshState();
    }

    WaterCheck(mPos, 1.0f);
}

// walkEffect() is deliberately defined inline in the header -- see the note there.

void daEnLkuriboBase_c::initializeState_Turn() {
    mSpeed.x = 0.0f;
    mDirection ^= 1;
}

void daEnLkuriboBase_c::finalizeState_Turn() {}

void daEnLkuriboBase_c::executeState_Turn() {
    mModel.play();
    mAnmTexPat.play();
    calcSpeedY();
    posMove();
    walkEffect();

    u32 bgCheck = EnBgCheck();

    if (bgCheck & 1) {
        mSpeed.y = 0.0f;
    }

    if (bgCheck & 4) {
        mPos.x = mLastPos.x;
    }

    WaterCheck(mPos, 1.0f);

    if (sLib::chaseAngle(&mAngle.y.mAngle, l_base_angleY[mDirection], 0x200)) {
        changeState(StateID_Walk);
    }
}

void daEnLkuriboBase_c::initializeState_Press() {
    setAnm("damage", m3d::FORWARD_ONCE, 0.0f);
    setTexAnm("damage", m3d::FORWARD_ONCE);

    mSpeed.set(0.0f, 0.0f, 0.0f);

    mAngle.y = 0;
    mNoRespawn = true;
}

void daEnLkuriboBase_c::finalizeState_Press() {}

void daEnLkuriboBase_c::executeState_Press() {
    mModel.play();
    mAnmTexPat.play();
    removeCc();

    if (mAnmChr.checkFrame(8.0f)) {
        split();
        deleteActor(1);
    }
}

void daEnLkuriboBase_c::split() {}

void daEnLkuriboBase_c::initializeState_Split() {
    setAnm("damage", m3d::FORWARD_ONCE, 0.0f);
    setTexAnm("damage", m3d::FORWARD_ONCE);

    mSpeed.set(0.0f, 0.0f, 0.0f);

    mAngle.y = 0;
    mNoRespawn = true;
}

void daEnLkuriboBase_c::finalizeState_Split() {}

void daEnLkuriboBase_c::executeState_Split() {
    mModel.play();
    mAnmTexPat.play();
    removeCc();
    pressAttach();

    if (mAnmChr.checkFrame(8.0f)) {
        split();
        deleteActor(1);
    }
}

void daEnLkuriboBase_c::initializeState_HipSplit() {
    setAnm("damage", m3d::FORWARD_ONCE, 0.0f);
    setTexAnm("damage", m3d::FORWARD_ONCE);

    mSpeed.set(0.0f, 0.0f, 0.0f);

    mAngle.y = 0;
    mNoRespawn = true;
}

void daEnLkuriboBase_c::finalizeState_HipSplit() {}

void daEnLkuriboBase_c::executeState_HipSplit() {
    mModel.play();
    mAnmTexPat.play();
    removeCc();
    pressAttach();

    if (mAnmChr.checkFrame(8.0f)) {
        hipsplit();
        deleteActor(1);
    }
}

void daEnLkuriboBase_c::hipsplit() {
    // RECONSTRUCTION, not original text. The target's .data holds a fourth
    // pointer-to-member constant {0, -1, &nonBoyoProc} at 0x80305104, directly
    // before the vtable, that nothing in the translation unit references -- the
    // only three setBoyoFunc call sites are accounted for by the constants at
    // 0x803050A8 (create), 0x803050EC (hitCallback_Fire) and 0x803050F8
    // (fireBoyoProc). So the original had a statement after fireBoyoProc that
    // materialises the constant without emitting code. This dead local
    // reproduces those 12 bytes and leaves hipsplit() a bare blr; it can be
    // moved into any function between fireBoyoProc and the vtable without
    // changing anything, since .data order follows source order.
    void (daEnLkuriboBase_c::*unused)() = &daEnLkuriboBase_c::nonBoyoProc;
}

void daEnLkuriboBase_c::initializeState_DieFall() {
    setAnm("split", m3d::FORWARD_LOOP, 0.0f);
    dEn_c::initializeState_DieFall();
}

void daEnLkuriboBase_c::finalizeState_DieFall() {}

void daEnLkuriboBase_c::executeState_DieFall() {
    mModel.play();
    mAnmTexPat.play();
    dEn_c::executeState_DieFall();
}

void daEnLkuriboBase_c::finalUpdate() {
    calcMdl();
    calcJnt();
}

void daEnLkuriboBase_c::setTurnByEnemyHit() {}

void daEnLkuriboBase_c::fumidamageSE(const mVec3_c &pos, int playerNo) {
    mVec3_c center = getCenterPos();
    int remote = dAudio::getRemotePlayer(playerNo);
    dAudio::SndObjctCmnEmy_c *obj = dAudio::g_pSndObjEmy;
    obj->startSound(SE_EMY_KURIBO_FUMU, center, remote);
}

bool daEnLkuriboBase_c::hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other) {
    return hitCallback_HipAttk(self, other);
}

daEnLkuriboBase_c::~daEnLkuriboBase_c() {}
