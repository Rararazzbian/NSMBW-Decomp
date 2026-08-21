#include <game/bases/d_game_com.hpp>
#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <game/bases/d_a_boss_demo.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_score_manager.hpp>
#include <game/framework/f_manager.hpp>
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
    mRootJntPos.z = 0.0f;
}

void dEnTorideKokoopa_c::calcShellJntPos() {
    mMdlShell.getNodeWorldMtxMultVecZero(mShellJntIdx, mShellJntPos);
    mShellJntPos.z = 0.0f;
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
        mVec3_c pos(mPos.x, mPos.y, 0.0f);
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
        mVec3_c pos(mPos.x, mPos.y, 0.0f);
        mEf::createEffect(ep->mJump, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::shellLandonEffect() {
    mVec3_c pos;
    pos.x = mPos.x;
    pos.y = mPos.y;
    pos.z = 0.0f;
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
        pos.z = 0.0f;
        mLevelEffect1.createEffect(ep->mHitFireLoop, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::hitFireDamageEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mHitFireDamage != nullptr) {
        mVec3_c pos;
        pos.x = mRootJntPos.x;
        pos.y = mRootJntPos.y;
        pos.z = 0.0f;
        mLevelEffect2.createEffect(ep->mHitFireDamage, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::shellChangeEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mShellChange != nullptr) {
        mVec3_c pos;
        pos.x = mShellJntPos.x;
        pos.y = mShellJntPos.y + getShellChangeEffectOffsetY();
        pos.z = 0.0f;
        mEf::createEffect(((EffectParam_t*)mUnkAF0)->mShellChange, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::fumidmgEffect() {
    EffectParam_t *ep = (EffectParam_t*)mUnkAF0;
    if (ep->mFumiDmg != nullptr) {
        mVec3_c pos = getCenterPos();
        pos.z = 0.0f;
        mEf::createEffect(((EffectParam_t*)mUnkAF0)->mFumiDmg, 0, &pos, nullptr, nullptr);
    }
}

void dEnTorideKokoopa_c::fumideadEffect() {
    const char *eff = isTorideBoss() ? ((EffectParam_t*)mUnkAF0)->mFumiDeadToride : ((EffectParam_t*)mUnkAF0)->mFumiDeadCastle;
    if (eff != nullptr) {
        mVec3_c pos = getCenterPos();
        pos.z = 0.0f;
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
    pos.z = 0.0f;
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

void dEnTorideKokoopa_c::finalizeState_FumiHit() {}
void dEnTorideKokoopa_c::finalizeState_FireHit() {}
void dEnTorideKokoopa_c::finalizeState_StarHit() {}
void dEnTorideKokoopa_c::finalizeState_SlideHit() {}
void dEnTorideKokoopa_c::finalizeState_ShellHit() {}

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
dEnTorideKokoopa_c::dEnTorideKokoopa_c() :
    mUnk600(0),
    mMdlKokoopa(),
    mAnmChrKokoopa(),
    mAnmMatClr(),
    mUnk6A8(0),
    mAnmTexPat(),
    mMdlShell(),
    mAnmChrShell(),
    mUnk750(0),
    mUnk754(0),
    mUnk758(0),
    mUnk75C(0),
    mUnk760(0),
    mUnk770(0),
    mUnk780(0),
    mUnk790(0),
    mUnk792(0),
    mUnk794(1),
    mCc(),
    mLevelEffect1(),
    mLevelEffect2(),
    mScaleSpeed(1.0f),
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


