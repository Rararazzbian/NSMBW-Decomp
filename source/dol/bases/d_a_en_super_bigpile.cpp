#include <game/bases/d_a_en_super_bigpile.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_quake.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/sLib/s_lib.hpp>
#include <constants/sound_list.h>

STATE_VIRTUAL_DEFINE(daEnSuperBigPile_c, GoWait);
STATE_VIRTUAL_DEFINE(daEnSuperBigPile_c, GoMove);
STATE_VIRTUAL_DEFINE(daEnSuperBigPile_c, RetWait);
STATE_VIRTUAL_DEFINE(daEnSuperBigPile_c, RetMove);

static const float l_speed_data[] = { 0.3f, 15.0f, 0.1f, 6.0f };

int daEnSuperBigPile_c::create() {
    createMdl();

    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

    mStartPos.x = mPos.x;
    mStartPos.y = mPos.y;
    mStartPos.z = mPos.z;

    mGoDist = ACTOR_PARAM(GoDist) * 16;
    mRetDist = ACTOR_PARAM(RetDist) * 16;

    if (mGoDist == 0) {
        mGoDist = 0x40;
    }
    if (mRetDist == 0) {
        mRetDist = 0x70;
    }

    initialize();

    mTimer1 = 60;
    changeState(StateID_GoWait);

    return SUCCEEDED;
}

void daEnSuperBigPile_c::initialize() {}

void daEnSuperBigPile_c::createMdl() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("daikonbou", "g3d/daikonbou.brres");
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("daikonbou_long");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::ANM_TEXSRT);
    dActor_c::setSoftLight_MapObj(mModel);

    mResAnmTexPat = mResFile.GetResAnmTexPat("daikonbou");
    mAnmTexPat.create(mdl, mResAnmTexPat, &mAllocator);
    mModel.setAnm(mAnmTexPat, 1.0f);
    mAnmTexPat.setAnm(mModel, mResAnmTexPat, 0, m3d::FORWARD_ONCE);

    if (ACTOR_PARAM(TexPattern) == 1) {
        mAnmTexPat.setFrame(1.0f, 0);
    } else {
        mAnmTexPat.setFrame(0.0f, 0);
    }

    mResAnmTexSrt = mResFile.GetResAnmTexSrt("daikonbou");
    mAnmTexSrt.create(mdl, mResAnmTexSrt, &mAllocator);
    mAnmTexSrt.setPlayMode(m3d::FORWARD_LOOP, 0);
    mModel.setAnm(mAnmTexSrt, 1.0f);
    mAnmTexSrt.setFrame(0.0f, 0);

    mAllocator.adjustFrmHeap();
}

int daEnSuperBigPile_c::execute() {
    mIsEffectCall = isEffectCall();
    mStateMgr.executeState();
    rolling();
    calcMdl();
    setCcInfo();
    return SUCCEEDED;
}

bool daEnSuperBigPile_c::isEffectCall() {
    return true;
}

void daEnSuperBigPile_c::setCcInfo() {}

int daEnSuperBigPile_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

void daEnSuperBigPile_c::deleteReady() {}

int daEnSuperBigPile_c::doDelete() {
    mCc.release();
    return SUCCEEDED;
}

void daEnSuperBigPile_c::postExecute(fBase_c::MAIN_STATE_e status) {
    if (status == SUCCESS) {
        mCc.clear();
    }
    dEn_c::postExecute(status);
}

void daEnSuperBigPile_c::calcMdl() {
    setTexRotate();

    mVec3_c pos = getDrawPos();
    mAng3_c angle = mAngle;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.ZrotM(angle.z);
    mMatrix.YrotM(angle.y);
    mMatrix.XrotM(angle.x);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daEnSuperBigPile_c::setTexRotate() {}

mVec3_c daEnSuperBigPile_c::getDrawPos() {
    return mPos;
}

void daEnSuperBigPile_c::rolling() {
    float dist = getMoveDist();
    s16 roll = dist * getDefaultRollSpeed();
    if (roll > 0xa00) {
        roll = 0xa00;
    }
    mAngle.y += roll;
}

float daEnSuperBigPile_c::getMoveDist() {
    return 0.0f;
}

s16 daEnSuperBigPile_c::getDefaultRollSpeed() const {
    return 0xa0;
}

void daEnSuperBigPile_c::calcFallSpeed() {
    sLib::chase(&mSpeedF, l_speed_data[1], l_speed_data[0]);
}

void daEnSuperBigPile_c::calcRiseSpeed() {
    sLib::chase(&mSpeedF, l_speed_data[3], l_speed_data[2]);
}

bool daEnSuperBigPile_c::PlDamageCheck(dCc_c *self, dCc_c *other) {
    return false;
}

bool daEnSuperBigPile_c::EtcDamageCheck(dCc_c *self, dCc_c *other) {
    if (other->mCcData.mAttack == CC_ATTACK_FIREBALL && hitCallback_Fire(self, other)) {
        return true;
    }
    if (other->mCcData.mAttack == CC_ATTACK_ICEBALL && hitCallback_Ice(self, other)) {
        return true;
    }
    return false;
}

bool daEnSuperBigPile_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    fireballInvalid(self, other);
    return false;
}

bool daEnSuperBigPile_c::hitCallback_Ice(dCc_c *self, dCc_c *other) {
    iceballInvalid(self, other);
    return false;
}

void daEnSuperBigPile_c::setQuake() {
    dQuake_c::m_instance->startShockAll(dQuake_c::TYPE_2, 1, 0, false);
}

void daEnSuperBigPile_c::fallDownSE() {
    mVec3_c pos;
    pos.x = dGameCom::getDispCenterX();
    pos.y = dGameCom::getDispCenterY();
    pos.z = mPos.z;
    dAudio::SoundEffectID_t(SE_OBJ_DAIKONBOU_LAND).playMapSound(pos, 0);
}

void daEnSuperBigPile_c::crashEffect() {
    mVec3_c pos(mPos.x, mPos.y, 5500.0f);
    mEf::createEffect("Wm_ob_cmnspark", 0, &pos, nullptr, nullptr);
}

void daEnSuperBigPile_c::initializeState_GoWait() {
    if (mIsEffectCall) {
        riseUpSE();
    }
}

void daEnSuperBigPile_c::riseUpSE() {}

void daEnSuperBigPile_c::finalizeState_GoWait() {}

void daEnSuperBigPile_c::executeState_GoWait() {
    if (mTimer1 == 0) {
        changeState(StateID_GoMove);
    }
}

void daEnSuperBigPile_c::initializeState_GoMove() {}

void daEnSuperBigPile_c::finalizeState_GoMove() {}

void daEnSuperBigPile_c::executeState_GoMove() {}

void daEnSuperBigPile_c::initializeState_RetWait() {
    if (mIsEffectCall) {
        fallDownSE();
        setQuake();
        if (ACTOR_PARAM(CrashEffect)) {
            crashEffect();
        }
    }
}

void daEnSuperBigPile_c::finalizeState_RetWait() {}

void daEnSuperBigPile_c::executeState_RetWait() {
    if (mTimer1 == 0) {
        changeState(StateID_RetMove);
    }
}

void daEnSuperBigPile_c::initializeState_RetMove() {}

void daEnSuperBigPile_c::finalizeState_RetMove() {}

void daEnSuperBigPile_c::executeState_RetMove() {}

float daEnSuperBigPile_c::getCamDist() {
    return 0.0f;
}

bool daEnSuperBigPile_c::YoshiDamageCheck(dCc_c *self, dCc_c *other) {
    return false;
}

bool daEnSuperBigPile_c::EnDamageCheck(dCc_c *self, dCc_c *other) {
    return false;
}

daEnSuperBigPile_c::~daEnSuperBigPile_c() {}
