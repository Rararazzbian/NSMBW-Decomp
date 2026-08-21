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

struct VoiceParam_t {
    u8 mPad[0x40];
    u32 mDamageSVo;
    u32 mPad44;
    u32 mDamageLVo;
};

void dEnTorideKokoopa_c::damageSVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mDamageSVo != 0x449) {
        mSound.startSound(vp->mDamageSVo, mSoundParam, 0);
    }
}

void dEnTorideKokoopa_c::damageLVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mDamageLVo != 0x449) {
        mSound.startSound(vp->mDamageLVo, mSoundParam, 0);
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
