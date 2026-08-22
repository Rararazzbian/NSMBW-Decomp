#include <game/bases/d_game_com.hpp>
#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <game/bases/d_a_boss_demo.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_score_manager.hpp>
#include <game/framework/f_manager.hpp>
#include <game/snd/snd_scene_manager.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/sLib/s_lib.hpp>

STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, Jump_St);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, Jump);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, BigJump_St);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, BigJump);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, LandOn);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, AttackReady);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, AttackBegin);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, AttackSearch);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, Attack);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, AttackEnd);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, FumiHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, FireHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, StarHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, SlideHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, QuakeHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, ShellHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, ShellAtk_St);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, ShellAtk);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, ShellOut);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DieFumi_St);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DieFire);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DieShell);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoWait);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoAwake);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoAwake_Wait);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoIkaku);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoIkaku_Wait);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoEscape_St);

int dEnTorideKokoopa_c::getTenmetsuTime_Fire() {
    return 24;
}

int dEnTorideKokoopa_c::getTenmetsuTime_Press() {
    return 24;
}

BOOL dEnTorideKokoopa_c::isFumiInvalid() const {
    return mUnk794 & 2;
}

BOOL dEnTorideKokoopa_c::isFireInvalid() const {
    return mUnk794 & 2;
}

BOOL dEnTorideKokoopa_c::isStarInvalid() const {
    return mUnk794 & 2;
}

void dEnTorideKokoopa_c::finalizeState_DemoWait() {}
void dEnTorideKokoopa_c::finalizeState_DieFire() {}
void dEnTorideKokoopa_c::finalizeState_DieShell() {}

mVec2_c dEnTorideKokoopa_c::getLookatPos() const {
    return mVec2_c(mLookatPos.x, mLookatPos.y);
}

void dEnTorideKokoopa_c::tenmetsuProc() {
    mAnmMatClr.play(0);
}

int dEnTorideKokoopa_c::preExecute() {
    if (!dEnBoss_c::preExecute()) {
        return 0;
    }
    if (*mStateMgr.getStateID() != StateID_FumiHit && *mStateMgr.getStateID() != StateID_DieFumi_St) {
        float sc = getDrawScale();
        mScale.x = sc;
        mScale.y = sc;
        mScale.z = sc;
    }
    moveAdjust_HIO();
    if (mpBossLife->mLife > 0) {
        sLib::chaseAngle(&mUnk790, mUnk792, 0x300);
    }
    return 1;
}

void dEnTorideKokoopa_c::postExecute(fBase_c::MAIN_STATE_e status) {
    if (status == 2) {
        if (mFireTimer > 0) {
            mFireTimer--;
            hitFireLoopEffect();
        }
        mCc.clear();
    }
    dEnBoss_c::postExecute(status);
}

int dEnTorideKokoopa_c::draw() {
    if (mUnk794 & 1) {
        drawKokoopa();
    }
    if (mUnk794 & 2) {
        drawShell();
    }
    return 1;
}

void dEnTorideKokoopa_c::setBattleReady() {
    setAtkCnt();
    changeState(StateID_AttackBegin);
}

void dEnTorideKokoopa_c::initializeState_DieFire() {
    dEnBoss_c::initializeState_DieFire();
    mTimer1 = 60;
}

void dEnTorideKokoopa_c::executeState_DieFire() {
    if (mTimer1 != 0) {
        hitFireDamageEffect();
    }
    dEnBoss_c::executeState_DieFire();
}

void dEnTorideKokoopa_c::initializeState_DieShell() {
    dEnBoss_c::initializeState_DieShell();
    mTimer1 = 60;
}

void dEnTorideKokoopa_c::executeState_DieShell() {
    if (mTimer1 != 0) {
        hitShellDamageEffect();
    }
    dEnBoss_c::executeState_DieShell();
}

void dEnTorideKokoopa_c::damageProc() {
    if (mpBossLife->isDmgSection()) {
        damageLVo();
    } else {
        damageSVo();
    }
    if (mpBossLife->isTwoDamage()) {
        speedUp();
    }
}

void dEnTorideKokoopa_c::deadProc() {
    deadVo();
    if (dActorMng_c::m_instance->mpBossDemo != nullptr) {
        dActorMng_c::m_instance->mpBossDemo->stopBGM();
    }
}






static const sDeathInfoData l_dieFumi_st = { 0.0f, 3.0f, -4.0f, -0.1875f, &dEnTorideKokoopa_c::StateID_DieFumi_St, -1, -1, 0, 0 };
static const sDeathInfoData l_dieFire = { 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieFire, -1, -1, 0, 0 };
static const sDeathInfoData l_dieStar = { 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0 };
static const sDeathInfoData l_dieQuake = { 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
static const sDeathInfoData l_dieShell = { 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieShell, -1, -1, 0, 0 };

void dEnTorideKokoopa_c::setFumiDead(dActor_c *killedBy) {
    u8 dir = (mPos.x < killedBy->mPos.x) ? 0 : 1;
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    int playerNo = killedBy->getPlrNo();
    if ((u32)playerNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, 6, playerNo, dScoreMng_c::smc_SCORE_X, dScoreMng_c::smc_SCORE_Y);
    }
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &StateID_DieFumi_St, -1, -1, 0, 0 };
    deathData.mDirection = dir;
    deathData.mKilledBy = killedBy->getPlrNo();
    mDeathInfo = deathData;
}

void dEnTorideKokoopa_c::setFireDead(dActor_c *killedBy) {
    u8 dir = (mPos.x < killedBy->mPos.x) ? 0 : 1;
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    int playerNo = killedBy->getPlrNo();
    if ((u32)playerNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, 6, playerNo, dScoreMng_c::smc_SCORE_X, dScoreMng_c::smc_SCORE_Y);
    }
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    mFireTimer = 60;
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieFire, -1, -1, 0, 0 };
    deathData.mDirection = dir;
    deathData.mKilledBy = killedBy->getPlrNo();
    mDeathInfo = deathData;
}

void dEnTorideKokoopa_c::setStarDead(dActor_c *killedBy) {
    u8 dir = (mPos.x < killedBy->mPos.x) ? 0 : 1;
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    int playerNo = killedBy->getPlrNo();
    if ((u32)playerNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, 6, playerNo, dScoreMng_c::smc_SCORE_X, dScoreMng_c::smc_SCORE_Y);
    }
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0 };
    deathData.mDirection = dir;
    deathData.mKilledBy = killedBy->getPlrNo();
    mDeathInfo = deathData;
}

void dEnTorideKokoopa_c::setQuakeDead() {
    static const sDeathInfoData deathData = {
        0.0f,
        3.0f,
        -4.0f,
        -0.1875f,
        &dEnBoss_c::StateID_DieStar,
        -1,
        -1,
        0,
        0xFF
    };
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    if (fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770)) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData death = deathData;
    death.mDirection = dir;
    mDeathInfo = death;
}

void dEnTorideKokoopa_c::setShellDead(dActor_c *killedBy) {
    u8 dir = (mPos.x < killedBy->mPos.x) ? 0 : 1;
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    int playerNo = killedBy->getPlrNo();
    if ((u32)playerNo <= 3) {
        dScoreMng_c::m_instance->ScoreSet(this, 6, playerNo, dScoreMng_c::smc_SCORE_X, dScoreMng_c::smc_SCORE_Y);
    }
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieShell, -1, -1, 0, 0 };
    deathData.mDirection = dir;
    deathData.mKilledBy = killedBy->getPlrNo();
    mDeathInfo = deathData;
}
void dEnTorideKokoopa_c::calcKokoopaMdl() {}
void dEnTorideKokoopa_c::calcShellMdl() {}
void dEnTorideKokoopa_c::drawKokoopa() {}
void dEnTorideKokoopa_c::drawShell() {}
void dEnTorideKokoopa_c::setKokoopaCc() {}
void dEnTorideKokoopa_c::setShellCc() {}
void dEnTorideKokoopa_c::moveAdjust_HIO() {}
void dEnTorideKokoopa_c::calcCcData() {}

float dEnTorideKokoopa_c::getDrawScale() {
    return 1.0f;
}

mVec3_c dEnTorideKokoopa_c::calcFacePos() {
    return mPos;
}

void dEnTorideKokoopa_c::calcRootJntPos() {
    mMdlKokoopa.getNodeWorldMtxMultVecZero(mRootJntIdx, mRootJntPos);
    mRootJntPos.z = 5500.0f;
}

void dEnTorideKokoopa_c::calcShellJntPos() {
    mMdlShell.getNodeWorldMtxMultVecZero(mShellJntIdx, mShellJntPos);
    mShellJntPos.z = 5500.0f;
}

void dEnTorideKokoopa_c::finalUpdate() {
    if (mUnk794 & 1) {
        calcKokoopaMdl();
        mBlitzPos = calcBlitzPos();
        mLookatPos = calcFacePos();
        calcCcData();
        calcRootJntPos();
    }
    if (mUnk794 & 2) {
        calcShellMdl();
        calcShellJntPos();
    }
}

float dEnTorideKokoopa_c::getJumpDist() const {
    return 64.0f;
}

float dEnTorideKokoopa_c::getKokoopaOffFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getShellOnFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getKokoopaOnFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getShellOffFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getCreateBlitzFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getShootFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getShellChangeEffectOffsetY() const {
    return 10.0f;
}

float dEnTorideKokoopa_c::getJumpGravity() {
    return -0.1875f;
}

mVec3_c dEnTorideKokoopa_c::getMagicStickEffectOffset() const {
    return mVec3_c(0.0f, 0.0f, 18.0f);
}

int dEnTorideKokoopa_c::getPressTime() { return 20; }
int dEnTorideKokoopa_c::defaultDirAngle() { return 0x2000; }
int dEnTorideKokoopa_c::getDownTime() { return 50; }
int dEnTorideKokoopa_c::getFumiRecoverTime() { return 4; }
int dEnTorideKokoopa_c::getAtkEndTime() { return 0; }
int dEnTorideKokoopa_c::getAtkEndTime_Wait() { return 0; }
int dEnTorideKokoopa_c::getAtkSearchTime() { return 0; }
int dEnTorideKokoopa_c::getAtkSearch2ndTime() { return 0; }
bool dEnTorideKokoopa_c::checkGetUp() const { return false; }


void dEnTorideKokoopa_c::changeShell() {
    setShellCc();
    shellChangeEffect();
    mAngle.x = 0;
}

void dEnTorideKokoopa_c::changeKokoopa() {
    setKokoopaCc();
    mAngle.x = 0;
}


void dEnTorideKokoopa_c::setBeginMoveState() {
    mUnk848 = dGameCom::rndInt(3) + 1;
    mAccelY = getJumpGravity();
    if (mUnk848 > 1) {
        changeState(StateID_Jump_St);
    } else {
        changeState(StateID_BigJump_St);
    }
}

void dEnTorideKokoopa_c::tenmetsuFin() {
    if (mAnmMatClr.mpChildren->getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 0);
    }
}




void dEnTorideKokoopa_c::jumpSE() {
    mSound.startSound(0x506, mSoundParam, 0);
}

void dEnTorideKokoopa_c::landonSE() {
    mSound.startSound(0x508, mSoundParam, 0);
}

void dEnTorideKokoopa_c::shellinSE() {
    mSound.startSound(0x50b, mSoundParam, 0);
}

void dEnTorideKokoopa_c::shellatkSE() {
    mSound.holdSound(0x50f, mSoundParam, 0);
}

void dEnTorideKokoopa_c::shelllandonSE() {
    mSound.startSound(0x50d, mSoundParam, 0);
}

void dEnTorideKokoopa_c::shelloutSE() {
    mSound.startSound(0x50c, mSoundParam, 0);
}

void dEnTorideKokoopa_c::blitzchargeSE() {}
void dEnTorideKokoopa_c::getupSE() {}
void dEnTorideKokoopa_c::awakeSE() {}
void dEnTorideKokoopa_c::ikakuSE() {}


struct VoiceEntry_t {
    u32 mSoundId;
    float mFrame;
};

struct VoiceParam_t {
    VoiceEntry_t mNotice1;
    VoiceEntry_t mNotice2[2];
    VoiceEntry_t mWake[2];
    u32 mEscJump;
    u32 mPad2C;
    u32 mMagicShot;
    u32 mPad34;
    VoiceEntry_t mShellOut;
    u32 mDamageS;
    u32 mPad44;
    u32 mDamageL;
    u32 mPad4C;
    u32 mDead;
    u32 mPad54;
    u32 mLoseFirst;
    u32 mPad5C;
    VoiceEntry_t mLoseSecond[3];
};

void dEnTorideKokoopa_c::notice1Vo() {
    if (*(u32*)(mUnk760 + 4) != 0) {
        VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
        if (vp != nullptr && vp->mNotice1.mSoundId != 0x449 && mAnmChrKokoopa.checkFrame(vp->mNotice1.mFrame)) {
            mSound.startSound(((VoiceParam_t*)mVoiceParam)->mNotice1.mSoundId, mSoundParam, 0);
        }
    }
}

void dEnTorideKokoopa_c::notice2Vo() {
    if (*(u32*)(mUnk760 + 0xc) != 0) {
        if (mVoiceParam != nullptr) {
            for (int i = 1; i <= 2; i++) {
                VoiceEntry_t *e = &((VoiceEntry_t*)mVoiceParam)[i];
                if (e->mSoundId != 0x449 && mAnmChrKokoopa.checkFrame(e->mFrame)) {
                    mSound.startSound(((VoiceEntry_t*)mVoiceParam)[i].mSoundId, mSoundParam, 0);
                }
            }
        }
    }
}

void dEnTorideKokoopa_c::wakeVo() {
    if (mVoiceParam != nullptr) {
        for (int i = 3; i <= 4; i++) {
            VoiceEntry_t *e = &((VoiceEntry_t*)mVoiceParam)[i];
            if (e->mSoundId != 0x449 && mAnmChrKokoopa.checkFrame(e->mFrame)) {
                mSound.startSound(((VoiceEntry_t*)mVoiceParam)[i].mSoundId, mSoundParam, 0);
            }
        }
    }
}

void dEnTorideKokoopa_c::escJumpVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mEscJump != 0x449) {
        mSound.startSound(vp->mEscJump, mSoundParam, 0);
    }
}

void dEnTorideKokoopa_c::magicShotVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mMagicShot != 0x449) {
        mSound.startSound(vp->mMagicShot, mSoundParam, 0);
    }
}

void dEnTorideKokoopa_c::shellOutVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mShellOut.mSoundId != 0x449 && mAnmChrKokoopa.checkFrame(vp->mShellOut.mFrame)) {
        mSound.startSound(((VoiceParam_t*)mVoiceParam)->mShellOut.mSoundId, mSoundParam, 0);
    }
}

void dEnTorideKokoopa_c::deadVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr) {
        if (vp->mDead != 0x449) {
            mSound.startSound(vp->mDead, mSoundParam, 0);
        } else if (vp->mDamageL != 0x449) {
            mSound.startSound(vp->mDamageL, mSoundParam, 0);
        }
    }
}

void dEnTorideKokoopa_c::loseFirstVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mLoseFirst != 0x449) {
        mSound.startSound(vp->mLoseFirst, mSoundParam, 0);
    }
}

void dEnTorideKokoopa_c::loseSecondVo() {
    if (mVoiceParam != nullptr) {
        for (int i = 12; i <= 14; i++) {
            VoiceEntry_t *e = &((VoiceEntry_t*)mVoiceParam)[i];
            if (e->mSoundId != 0x449 && mAnmChrKokoopa.checkFrame(e->mFrame)) {
                mSound.startSound(((VoiceEntry_t*)mVoiceParam)[i].mSoundId, mSoundParam, 0);
            }
        }
    }
}

void dEnTorideKokoopa_c::damageSVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mDamageS != 0x449) {
        mSound.startSound(vp->mDamageS, mSoundParam, 0);
    }
}

void dEnTorideKokoopa_c::damageLVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mDamageL != 0x449) {
        mSound.startSound(vp->mDamageL, mSoundParam, 0);
    }
}

struct EffectParam_t {
    const char *mName;
    const char *mJump;
    const char *mFumiDmg;
    const char *mHitFireLoop;
    const char *mLandOn;
    const char *mHitFireDamage;
    const char *mShellChange;
    const char *mShellBumMar;
    const char *mDownLandOn[2];
    const char *mShellAtk[2];
    const char *mShellAtkFast[2];
    const char *mShellAtkLoop;
    const char *mFumiDeadToride;
    const char *mFumiDeadCastle;
    const char *mShellWall;
    const char *mDownFall;
};

void dEnTorideKokoopa_c::jumpEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mJump != nullptr) {
        mVec3_c pos(mPos.x, mPos.y, 5500.0f);
        mEf::createEffect(ep->mJump, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::jumpRootEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mShellBumMar != nullptr) {
        mVec3_c pos;
        pos.x = mRootJntPos.x;
        pos.y = mRootJntPos.y;
        pos.z = mPos.z - 64.0f;
        mLevelEffects[0].createEffect(ep->mShellBumMar, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::landonEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mJump != nullptr) {
        mVec3_c pos(mPos.x, mPos.y, 5500.0f);
        mEf::createEffect(ep->mJump, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::shellLandonEffect() {
    mVec3_c pos;
    pos.x = mPos.x;
    pos.y = mPos.y;
    pos.z = 5500.0f;
    if (((EffectParam_t*)mUnkAF0)->mDownLandOn[0] != nullptr) {
        mEf::createEffect(((EffectParam_t*)mUnkAF0)->mDownLandOn[0], 0, &pos, nullptr, nullptr);
    }
    if (((EffectParam_t*)mUnkAF0)->mDownLandOn[1] != nullptr) {
        mEf::createEffect(((EffectParam_t*)mUnkAF0)->mDownLandOn[1], 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::hitFireLoopEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mHitFireLoop != nullptr) {
        mVec3_c pos;
        pos.x = mRootJntPos.x;
        pos.y = mRootJntPos.y;
        pos.z = 5500.0f;
        mLevelEffect1.createEffect(ep->mHitFireLoop, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::hitFireDamageEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mHitFireDamage != nullptr) {
        mVec3_c pos;
        pos.x = mRootJntPos.x;
        pos.y = mRootJntPos.y;
        pos.z = 5500.0f;
        mLevelEffect2.createEffect(ep->mHitFireDamage, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::shellChangeEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mShellChange != nullptr) {
        mVec3_c pos;
        pos.x = mShellJntPos.x;
        pos.y = mShellJntPos.y + getShellChangeEffectOffsetY();
        pos.z = 5500.0f;
        mEf::createEffect(((EffectParam_t*)mUnkAF0)->mShellChange, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::fumidmgEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mFumiDmg != nullptr) {
        mVec3_c pos = getCenterPos();
        pos.z = 5500.0f;
        mEf::createEffect(((EffectParam_t*)mUnkAF0)->mFumiDmg, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::fumideadEffect() {
    const char *eff = isTorideBoss() ? ((EffectParam_t*)mUnkAF0)->mFumiDeadToride : ((EffectParam_t*)mUnkAF0)->mFumiDeadCastle;
    if (eff != nullptr) {
        mVec3_c pos = getCenterPos();
        pos.z = 5500.0f;
        mEf::createEffect(eff, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::downFallEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mDownFall != nullptr) {
        mVec3_c pos;
        pos.x = mRootJntPos.x;
        pos.y = mRootJntPos.y;
        pos.z = mPos.z - 64.0f;
        mLevelEffects[0].createEffect(ep->mDownFall, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::downLandOnEffect(float scale) {
    mVec3_c pos;
    pos.x = mPos.x;
    pos.y = mPos.y;
    pos.z = 5500.0f;
    mVec3_c sc(scale, scale, scale);
    if (((EffectParam_t*)mUnkAF0)->mDownLandOn[0] != nullptr) {
        mEf::createEffect(((EffectParam_t*)mUnkAF0)->mDownLandOn[0], 0, &pos, nullptr, &sc);
    }
    if (((EffectParam_t*)mUnkAF0)->mDownLandOn[1] != nullptr) {
        mEf::createEffect(((EffectParam_t*)mUnkAF0)->mDownLandOn[1], 0, &pos, nullptr, &sc);
    }
}

mVec3_c dEnTorideKokoopa_c::getPressScale() {
    return mVec3_c(1.8f, 1.0f, 1.2f);
}

void dEnTorideKokoopa_c::hitShellDamageEffect() {}
void dEnTorideKokoopa_c::ikakuEffect() {}

void dEnTorideKokoopa_c::finalizeState_Jump_St() {}
void dEnTorideKokoopa_c::finalizeState_Jump() {}
void dEnTorideKokoopa_c::finalizeState_BigJump_St() {}
void dEnTorideKokoopa_c::finalizeState_BigJump() {}
void dEnTorideKokoopa_c::finalizeState_LandOn() {}
void dEnTorideKokoopa_c::finalizeState_AttackReady() {}
void dEnTorideKokoopa_c::finalizeState_AttackBegin() {}
void dEnTorideKokoopa_c::finalizeState_AttackSearch() {}
void dEnTorideKokoopa_c::finalizeState_Attack() {}

void dEnTorideKokoopa_c::finalizeState_AttackEnd() {
    mCc.release();
}

void dEnTorideKokoopa_c::initializeState_FumiHit() {
    const char *anmName = mpParamShell->mAnmNames[1];
    if (anmName != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)0);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 0.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    removeCc();
    mCc.release();
    mCc.release();
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    mAngle.x = 0;
    mAngle.y = 0;
    mScale = mUnkAA4 = getPressScale();
    mTimer1 = getPressTime();
    m_23b = 1;
}

void dEnTorideKokoopa_c::executeState_FumiHit() {
    mMdlKokoopa.play();
    mBc.checkFootEnm();
    float sc = getDrawScale();
    switch (m_23b) {
    case 1:
        if (mTimer1 == 0) {
            mVec3_c diff(sc - mUnkAA4.x, sc - mUnkAA4.y, sc - mUnkAA4.z);
            int rec = getFumiRecoverTime();
            if (rec <= 0) {
                mUnkAB0 = diff;
            } else {
                float inv = 1.0f / (float)rec;
                mVec3_c step(diff.x * inv, diff.y * inv, diff.z * inv);
                mUnkAB0 = step;
            }
            m_23b = 2;
        }
        break;
    case 2:
        mScale.x += mUnkAB0.x;
        mScale.y += mUnkAB0.y;
        mScale.z += mUnkAB0.z;
        if (mScale.y >= sc) {
            mScale.x = sc;
            mScale.y = sc;
            mScale.z = sc;
            changeState(StateID_ShellAtk_St);
            mStateMgr.executeState();
        }
        break;
    }
}

void dEnTorideKokoopa_c::finalizeState_FumiHit() {}
void dEnTorideKokoopa_c::finalizeState_FireHit() {}
void dEnTorideKokoopa_c::finalizeState_StarHit() {}
void dEnTorideKokoopa_c::finalizeState_SlideHit() {}
void dEnTorideKokoopa_c::finalizeState_ShellHit() {}

void dEnTorideKokoopa_c::initializeState_Jump() {
    const char *anmName = mpParamJump->mAnmNames[1];
    if (anmName != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 0.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    int flag = 1;
    if (mpBossLife->isNonDamage() == 0 && mpBossLife->isOneDamage() == 0) {
        flag = 0;
    }
    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mJumpSpeed1;
    } else {
        speed = mpParamJump->mJumpSpeed2;
    }
    float rate = calcJumpRate();
    f32 sy = speed.y;
    f32 sx = speed.x;
    mSpeed.y = sy;
    mSpeed.x = (float)l_EnMuki[mDirection] * rate * sx;
    jumpEffect();
    jumpSE();
}

void dEnTorideKokoopa_c::initializeState_BigJump() {
    const char *anmName = mpParamJump->mAnmNames[3];
    if (anmName != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 0.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    int flag = 1;
    if (mpBossLife->isNonDamage() == 0 && mpBossLife->isOneDamage() == 0) {
        flag = 0;
    }
    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mBigJumpSpeed1;
    } else {
        speed = mpParamJump->mBigJumpSpeed2;
    }
    float rate = calcJumpRate();
    f32 sy = speed.y;
    f32 sx = speed.x;
    mSpeed.y = sy;
    mSpeed.x = (float)l_EnMuki[mDirection] * rate * sx;
    jumpEffect();
    jumpSE();
}

void dEnTorideKokoopa_c::executeState_Jump_St() {
    executeState_StarHit();
}

void dEnTorideKokoopa_c::executeState_QuakeHit() {
    executeState_StarHit();
}

void dEnTorideKokoopa_c::finalizeState_QuakeHit() {
    finalizeState_StarHit();
}

void dEnTorideKokoopa_c::finalizeState_ShellAtk_St() {
    reviveCc();
}

void dEnTorideKokoopa_c::finalizeState_ShellAtk() {
    mActorProperties |= 0x200;
}

void dEnTorideKokoopa_c::finalizeState_ShellOut() {}
void dEnTorideKokoopa_c::finalizeState_DieFumi_St() {}
void dEnTorideKokoopa_c::finalizeState_DemoAwake() {}
void dEnTorideKokoopa_c::finalizeState_DemoAwake_Wait() {}
void dEnTorideKokoopa_c::finalizeState_DemoIkaku() {}
void dEnTorideKokoopa_c::finalizeState_DemoIkaku_Wait() {}

void dEnTorideKokoopa_c::initializeState_DemoEscape_St() {}
void dEnTorideKokoopa_c::executeState_DemoEscape_St() {}
void dEnTorideKokoopa_c::finalizeState_DemoEscape_St() {}

int dEnTorideKokoopa_c::beginDance() { return 0; }
int dEnTorideKokoopa_c::createBlitz() { return 0; }
void dEnTorideKokoopa_c::speedUp() {}
void dEnTorideKokoopa_c::calcWandCcData() {}
void dEnTorideKokoopa_c::setBlitzTarget() {}
void dEnTorideKokoopa_c::blitzShoot() {}
int dEnTorideKokoopa_c::getTorideFunfareTime() { return 40; }
static const float l_bounceSpeed[] = { 0.0f, 1.5f, 2.75f, 4.0f };

void dEnTorideKokoopa_c::initializeState_ShellAtk_St() {
    if (mUnk794 == 1) {
        const char *anmKokoopa = mpParamShell->mAnmNames[2];
        if (anmKokoopa != nullptr) {
            nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmKokoopa);
            mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
            mMdlKokoopa.setAnm(mAnmChrKokoopa, 4.0f);
            mAnmChrKokoopa.setRate(1.0f);
        }
        const char *anmShell = mpParamShell->mAnmNames[4];
        if (anmShell != nullptr) {
            nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmShell);
            mAnmChrShell.setAnm(mMdlShell, anm, (m3d::playMode_e)1);
            mMdlShell.setAnm(mAnmChrShell, 0.0f);
            mAnmChrShell.setRate(1.0f);
        }
    }
    if (*mStateMgr.getOldStateID() != StateID_FireHit &&
        *mStateMgr.getOldStateID() != StateID_SlideHit &&
        *mStateMgr.getOldStateID() != StateID_StarHit)
    {
        mAccelY = -0.275f;
        if (checkDownJump()) {
            mSpeed.set(0.0f, 6.25f, 0.0f);
        } else {
            mSpeed.set(0.0f, 5.25f, 0.0f);
        }
        mSpeedMax.y = -4.0f;
    }
    mUnkAC0 = 0x200;
    mUnk792 = 0;
    mUnkAA0 = 4;
    mUnk794 |= 2;
    mActorProperties &= ~8;
    shellinSE();
}

void dEnTorideKokoopa_c::executeState_ShellAtk_St() {
    static const float l_bounceSpeed[] = { 0.0f, 1.5f, 2.75f, 4.0f };
    mMdlKokoopa.play();
    mMdlShell.play();
    if (mAnmChrKokoopa.isStop()) {
        if (mUnk794 != 2) {
            changeShell();
            mUnk794 = 2;
        }
    }
    if (*mStateMgr.getOldStateID() == StateID_FireHit ||
        *mStateMgr.getOldStateID() == StateID_SlideHit)
    {
        if (mSpeed.y >= 0.0f || mUnk794 == 2) {
            calcSpeedY();
            posMove();
        }
    } else {
        calcSpeedY();
        posMove();
    }
    if (mUnk794 == 2) {
        mUnkAC0 += 0x200;
        if (mUnkAC0 > 0x1800) {
            mUnkAC0 = 0x1800;
        }
        mAngle.y += mUnkAC0;
        shellatkSE();
        shellBumMarEffect();
    } else {
        if (*mStateMgr.getOldStateID() == StateID_FireHit) {
            hitFireDamageEffect();
        }
    }
    mAngle.z = 0;
    if (mBc.checkFootEnm()) {
        shelllandonSE();
        shellLandonEffect();
        if (--mUnkAA0 > 0) {
            mSpeed.y = l_bounceSpeed[mUnkAA0];
        } else {
            mSpeed.y = 0.0f;
            changeState(StateID_ShellAtk);
        }
    }
}

static const float l_shellatk_speed[2] = { 4.0f, -4.0f };

void dEnTorideKokoopa_c::executeState_AttackSearch() {
    mMdlKokoopa.play();
    if (mpParamAttack->mPad10 != 0) {
        mAnmMatClr.play(1);
    }
    calcSpeedY();
    posMove();
    if (mBc.checkFootEnm()) {
        mSpeed.y = 0.0f;
    }
    mUnk764 = calcAttackTarget();
    bool lockon = lockonTurn();
    calcLookAngle();
    blitzchargeSE();
    switch (m_23b) {
    case 1:
        if (isCreateBlitz()) {
            mUnk770 = createBlitz();
            if (mpBossLife->isTwoDamage() && mAtkCnt == 1) {
                mUnk768 = (s16)getAtkSearch2ndTime();
            } else {
                mUnk768 = (float)(s16)getAtkSearchTime() / mUnkACC;
            }
            m_23b = 2;
        }
        break;
    case 2:
        blitzMove((dActor_c*)(mUnk770 == 0 ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770)));
        if (--mUnk768 <= 0) {
            if (lockon) {
                changeState(StateID_Attack);
            }
        }
        break;
    }
}

void dEnTorideKokoopa_c::executeState_Attack() {
    mMdlKokoopa.play();
    calcSpeedY();
    posMove();
    if (mBc.checkFootEnm()) {
        mSpeed.y = 0.0f;
    }
    dActor_c *blitz = (mUnk770 == 0) ? (dActor_c*)nullptr : (dActor_c*)fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (blitz == nullptr) {
        m_23b = 2;
    }
    switch (m_23b) {
    case 1:
        blitzMove(blitz);
        if (isShootBlitz()) {
            blitz->mDirection = mDirection;
            calcWandCcData();
            setBlitzTarget();
            blitzShoot();
            magicShotVo();
            mUnk770 = 0;
            if (mpParamAttack->mPad10 != 0) {
                mAnmMatClr.setFrame(0.0f, 1);
            }
            m_23b = 2;
        }
        break;
    case 2:
        if (mAnmChrKokoopa.isStop()) {
            if (--mAtkCnt > 0) {
                changeState(StateID_AttackBegin);
            } else {
                changeState(StateID_AttackEnd);
            }
        }
        break;
    }
}

void dEnTorideKokoopa_c::initializeState_ShellAtk() {
    float left = dGameCom::getDispCenterX() + mUnk840;
    float right = dGameCom::getDispCenterX() + mUnk844;
    float center = 0.5f * (left + right);
    if (mPos.x >= center) {
        mDirection = 1;
    } else {
        mDirection = 0;
    }
    mActorProperties &= ~0x200;
    mUnkAC8 = 5;
    mAccelF = 0.3f;
    mSpeed.x = l_shellatk_speed[mDirection];
    if (mPos.x - 32.0f < right) {
        mSpeedMax.x = mSpeed.x;
        mUnkAC4 = 32.0f + right;
        mUnkAC8 = 6;
    } else if (mPos.x + 32.0f > left) {
        mSpeedMax.x = mSpeed.x;
        mUnkAC4 = left - 32.0f;
        mUnkAC8 = 6;
    } else {
        mUnkAC4 = mPos.x;
        mSpeedMax.x = l_shellatk_speed[mDirection ^ 1];
    }
}

void dEnTorideKokoopa_c::executeState_ShellAtk() {
    static const s16 cs_atkangle_z[2] = { 0x038E, -0x038E };
    calcSpeedX();
    calcSpeedY();
    f32 lastX = mPos.x;
    posMove();
    if (mBc.checkFootEnm()) {
        mSpeed.y = 0.0f;
    }
    if (mSpeed.x >= 0.0f) {
        mDirection = 0;
    } else {
        mDirection = 1;
    }
    s16 targetAngle = 0;
    mAngle.y += mUnkAC0;
    if (mUnkAC8 > 1) {
        targetAngle = -cs_atkangle_z[mDirection];
    }
    sLib::chaseAngle((s16*)&mAngle.z, targetAngle, 0x80);
    shellatkSE();
    shellBumMarEffect();
    shellAtkEffect();
    if ((lastX < mUnkAC4 && mPos.x >= mUnkAC4) || (lastX > mUnkAC4 && mPos.x <= mUnkAC4)) {
        if (--mUnkAC8 > 0) {
            mSpeedMax.x = -mSpeedMax.x;
        } else {
            mSpeed.x = 0.0f;
            mDirection = getPl_LRflag(mPos);
            s32 dir = defaultDirAngle();
            s8 muki = l_EnMuki[mDirection];
            mAngle.z = 0;
            mAngle.y = muki * dir;
            changeState(StateID_ShellOut);
        }
    }
}

void dEnTorideKokoopa_c::initializeState_ShellOut() {
    const char *anmKokoopa = mpParamShell->mAnmNames[3];
    if (anmKokoopa != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmKokoopa);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 1.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    const char *anmShell = mpParamShell->mAnmNames[5];
    if (anmShell != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmShell);
        mAnmChrShell.setAnm(mMdlShell, anm, (m3d::playMode_e)1);
        mMdlShell.setAnm(mAnmChrShell, 1.0f);
        mAnmChrShell.setRate(1.0f);
    }
    mSpeedMax.x = 0.0f;
    mAccelF = 0.0f;
    mSpeed.set(0.0f, 0.0f, 0.0f);
    mActorProperties |= 8;
    shelloutSE();
    shellOutVo();
    if (mpBossLife->isTwoDamage()) {
        SndSceneMgr::sInstance->fn_8019bd90(0x800);
    }
    m_23b = 1;
}

void dEnTorideKokoopa_c::executeState_ShellOut() {
    mMdlKokoopa.play();
    mMdlShell.play();
    calcSpeedY();
    posMove();
    if (mBc.checkFootEnm()) {
        mSpeed.y = 0.0f;
    }
    if (checkGetUp()) {
        getupSE();
    }
    shellOutVo();
    switch (m_23b) {
    case 1:
        if (!(mUnk794 & 1)) {
            if (mAnmChrKokoopa.checkFrame(getKokoopaOnFrm())) {
                mUnk794 |= 1;
            }
        }
        if (mUnk794 & 2) {
            if (mAnmChrKokoopa.checkFrame(getShellOffFrm())) {
                mUnk794 &= ~2;
            }
        }
        if (mUnk794 == 1) {
            changeKokoopa();
            m_23b = 2;
        }
        break;
    case 2:
        if (mAnmChrKokoopa.isStop()) {
            setBeginMoveState();
        }
        break;
    }
}

bool KokoopaSpFumiCheck_c::operate(int &result, dEn_c *en, FumiCcInfo_c &fumi) {
    result = 0;
    daPlBase_c *player = (daPlBase_c*)fumi.mCc2->mpOwner;
    if ((*(u32*)((char*)player + 0x1074) | *(u32*)((char*)player + 0x1078)) != 0) {
        if (player->mSpeed.y > 0.0f) {
            result = 0;
            return true;
        }
    }
    if (!player->mBc.isFoot() && en->mSpeed.y > 0.0f) {
        if (*(s32*)((char*)player + 0x1090) == 3) {
            if (player->mPos.y >= en->mPos.y + 4.0f) {
                int plrNo = player->getPlrNo();
                en->mNoHitPlayer.mTimer[plrNo] = 24;
                result = 1;
                return true;
            }
        } else {
            if (player->mPos.y >= en->mPos.y + 10.0f) {
                int plrNo = player->getPlrNo();
                en->mNoHitPlayer.mTimer[plrNo] = 24;
                result = 1;
                return true;
            }
        }
    }
    return false;
}

void dEnTorideKokoopa_c::initializeState_DieFumi_St() {
    const char *anmName = mpParamShell->mAnmNames[1];
    if (anmName != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)0);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 3.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    removeCc();
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    mAngle.x = 0;
    mAngle.y = 0;
    mScale = mUnkAA4 = getPressScale();
    mTimer1 = getPressTime();
    m_23b = 1;
}

void dEnTorideKokoopa_c::executeState_DieFumi_St() {
    mMdlKokoopa.play();
    mBc.checkFootEnm();
    float sc = getDrawScale();
    switch (m_23b) {
    case 1:
        if (mTimer1 == 0) {
            mVec3_c diff(sc - mUnkAA4.x, sc - mUnkAA4.y, sc - mUnkAA4.z);
            int rec = getFumiRecoverTime();
            if (rec <= 0) {
                mUnkAB0 = diff;
            } else {
                float inv = 1.0f / (float)rec;
                mVec3_c step(diff.x * inv, diff.y * inv, diff.z * inv);
                mUnkAB0 = step;
            }
            m_23b = 2;
        }
        break;
    case 2:
        mScale.x += mUnkAB0.x;
        mScale.y += mUnkAB0.y;
        mScale.z += mUnkAB0.z;
        if (mScale.y >= sc) {
            mScale.x = sc;
            mScale.y = sc;
            mScale.z = sc;
            changeState(dEnBoss_c::StateID_DieFumi);
        }
        break;
    }
}

dEnTorideKokoopa_c::dEnTorideKokoopa_c() :
    mResFile(),
    mMdlKokoopa(),
    mAnmChrKokoopa(),
    mAnmMatClr(),
    mUnk6A8(0),
    mAnmTexPat(),
    mMdlShell(),
    mAnmChrShell(),
    mpParamReady(0),
    mpParamJump(0),
    mpParamAttack(0),
    mpParamShell(0),
    mUnk760(0),
    mUnk770(0),
    mUnk780(0),
    mUnk790(0),
    mUnk792(0),
    mUnk794(1),
    mCc(),
    mLevelEffect1(),
    mLevelEffect2(),
    mUnkACC(1.0f),
    mRootJntIdx(-1),
    mShellJntIdx(-1),
    mUnkAF0(0)
{
    mVoiceParam = nullptr;
    mLookatPos = mPos;
    mFumiProc.mFumiCheck.m_00 = 5;
    mFumiProc.refresh(new KokoopaSpFumiCheck_c());
}

dEnTorideKokoopa_c::~dEnTorideKokoopa_c() {
    mCc.release();
}



