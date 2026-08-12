#include <game/bases/d_a_fireball_base.hpp>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_cd.hpp>
#include <game/bases/d_ef.hpp>
#include <game/bases/d_eff_actor_manager.hpp>
#include <game/bases/d_effectmanager.hpp>
#include <game/bases/d_s_stage.hpp>
#include <constants/sound_list.h>

const float daFireBall_Base_c::smc_MAXFALLSPEED = -4.0f;
const float daFireBall_Base_c::smc_GRAVITY = -0.4375f;

extern const sBcSensorPoint l_fireball_foot;
extern const sBcSensorPoint l_fireball_head;
extern const sBcSensorPoint l_fireball_wall;

const sBcSensorPoint l_fireball_foot = { SENSOR_IS_POINT, 0, -0x3000 };
const sBcSensorPoint l_fireball_head = { SENSOR_IS_POINT, 0, 0x3000 };
const sBcSensorPoint l_fireball_wall = { SENSOR_IS_POINT, 0x3000, 0 };

const float l_cull_data[4] = { 0.0f, 0.0f, 8.0f, 8.0f };

STATE_VIRTUAL_DEFINE(daFireBall_Base_c, Move);
STATE_VIRTUAL_DEFINE(daFireBall_Base_c, Kill);
STATE_VIRTUAL_DEFINE(daFireBall_Base_c, EatIn);
STATE_VIRTUAL_DEFINE(daFireBall_Base_c, EatNow);

int daFireBall_Base_c::create() {
    if (!createCheck()) {
        deleteRequest();
        return CANCELED;
    }

    if (!initialize()) {
        deleteRequest();
        return CANCELED;
    }

    mAreaNo = dCd_c::m_instance->getFileP(dScStage_c::m_instance->mCurrFile)->getAreaNo(&mPos);

    setCc();
    setBc();

    mActorProperties |= 0x80;
    mCenterOffs.set(0.0f, 0.0f, 0.0f);
    mEatBehavior = EAT_TYPE_FIREBALL;

    mLightMask.init(&mAllocator, 2);

    mLiquidType = dBc_c::checkWater(mPos.x, mPos.y, mLayer, &mLiquidHeight);
    mStartPos = mPos;

    mStateMgr.changeState(StateID_Move);

    return SUCCEEDED;
}

int daFireBall_Base_c::createCheck() {
    return true;
}

int daFireBall_Base_c::initialize() {
    return true;
}

void daFireBall_Base_c::setCc() {}

int daFireBall_Base_c::preExecute() {
    if (!dActor_c::preExecute()) {
        return CANCELED;
    }

    if (mDeleteRequested) {
        return CANCELED;
    }

    if (mIsDead) {
        if (!isState(StateID_Kill)) {
            mStateMgr.changeState(StateID_Kill);
        }
    }

    return SUCCEEDED;
}

int daFireBall_Base_c::execute() {
    mStateMgr.executeState();
    chgZpos();

    if (mEatState != EAT_STATE_EATEN && !isState(StateID_Kill)) {
        fireEffect();
        lightProc();

        int prevLiquid = mLiquidType;
        mLiquidType = dBc_c::checkWater(mPos.x, mPos.y, mLayer, &mLiquidHeight);

        if (mLiquidType == dBc_c::WATER_CHECK_WATER) {
            if (prevLiquid == dBc_c::WATER_CHECK_NONE) {
                beginSplash(mLiquidHeight);
            }
        } else if (mLiquidType == dBc_c::WATER_CHECK_WATER_BUBBLE) {
            if (prevLiquid == dBc_c::WATER_CHECK_NONE) {
                beginSplash(mPos.y);
            }
        } else if (mLiquidType == dBc_c::WATER_CHECK_YOGAN) {
            if (prevLiquid == dBc_c::WATER_CHECK_NONE) {
                beginYoganSplash(mLiquidHeight);
            }
        } else if (mLiquidType == dBc_c::WATER_CHECK_POISON) {
            if (prevLiquid == dBc_c::WATER_CHECK_NONE) {
                beginPoisonSplash(mLiquidHeight);
            }
        }
    }

    if (cullCheck()) {
        mIsDead = 1;
    }

    return SUCCEEDED;
}

void daFireBall_Base_c::chgZpos() {}

void daFireBall_Base_c::fireEffect() {
    EffectManager_c::SetFireBallEffect(&mPos);
}

int daFireBall_Base_c::draw() {
    mLightMask.draw();
    return SUCCEEDED;
}

void daFireBall_Base_c::deleteReady() {}

int daFireBall_Base_c::doDelete() {
    return SUCCEEDED;
}

bool daFireBall_Base_c::cullCheck() {
    if (dActor_c::screenCullCheck(mPos, (const sRangeDataF &) l_cull_data, sRangeDataF(64.0f, 64.0f, 32.0f, 32.0f), mAreaNo)) {
        goto culled;
    }
    if (areaCullCheck(mPos, (const sRangeDataF &) l_cull_data, mAreaNo)) {
        goto culled;
    }
    goto notCulled;
culled:
    return true;
notCulled:
    return false;
}

void daFireBall_Base_c::setBc() {
    mBc.set(this, l_fireball_foot, l_fireball_head, l_fireball_wall);
    mBc.mpRc = &mRc;
}

void daFireBall_Base_c::kill() {
    dEf::createEffect_change("Wm_mr_fireball_hit", 0, &mPos, nullptr, nullptr);
    mIsDead = 1;
}

void daFireBall_Base_c::setEatTongue(dActor_c *eatingActor) {
    removeCc();
    mStateMgr.changeState(StateID_EatIn);
}

void daFireBall_Base_c::lightProc() {
    mVec3_c pos(mPos.x, mPos.y, mPos.z);
    float rad = getLightRad();

    mLightMask.mPos = pos;
    mLightMask.mRadius = rad;
    mLightMask.execute();
}

float daFireBall_Base_c::getLightRad() const {
    return 120.0f;
}

void daFireBall_Base_c::beginSplash(float height) {
    dAudio::SoundEffectID_t(SE_OBJ_EMY_FIRE_SPLASH).playMapSound(mPos, 0);

    mVec3_c pos(mPos.x, height, 5500.0f);
    u32 splashFlags = mLayer << 16;

    dEffActorMng_c::m_instance->createWaterSplashEff(pos, splashFlags | 2, -1, mVec3_c(1.0f, 1.0f, 1.0f));
    dEffActorMng_c::m_instance->createWaterSplashEff(pos, splashFlags | 3, -1, mVec3_c(1.0f, 1.0f, 1.0f));

    dBg_c::m_bg_p->setWaterInWave(mPos.x, mPos.y, 8);

    mIsDead = 1;
}

void daFireBall_Base_c::beginYoganSplash(float height) {
    dAudio::SoundEffectID_t(SE_OBJ_CMN_SPLASH_LAVA).playMapSound(mPos, 0);

    mVec3_c pos(mPos.x, height, 5500.0f);
    u32 splashFlags = mLayer << 16 | 4;
    mVec3_c scale(0.6f, 0.6f, 0.6f);

    dEffActorMng_c::m_instance->createWaterSplashEff(pos, splashFlags, -1, scale);

    dBg_c::m_bg_p->setWaterInWave(mPos.x, mPos.y, 18);

    mIsDead = 1;
}

void daFireBall_Base_c::beginPoisonSplash(float height) {
    dAudio::SoundEffectID_t(SE_OBJ_CMN_SPLASH_POISON).playMapSound(mPos, 0);

    mVec3_c pos(mPos.x, height, 5500.0f);
    u32 splashFlags = mLayer << 16 | 6;
    mVec3_c scale(0.6f, 0.6f, 0.6f);

    dEffActorMng_c::m_instance->createWaterSplashEff(pos, splashFlags, -1, scale);

    dBg_c::m_bg_p->setWaterInWave(mPos.x, mPos.y, 25);

    mIsDead = 1;
}

void daFireBall_Base_c::initializeState_Move() {}

void daFireBall_Base_c::finalizeState_Move() {}

void daFireBall_Base_c::executeState_Move() {}

void daFireBall_Base_c::initializeState_EatIn() {}

void daFireBall_Base_c::finalizeState_EatIn() {}

void daFireBall_Base_c::executeState_EatIn() {
    if (mEatState == EAT_STATE_EATEN) {
        mStateMgr.changeState(StateID_EatNow);
    }
}

void daFireBall_Base_c::initializeState_EatNow() {}

void daFireBall_Base_c::finalizeState_EatNow() {}

void daFireBall_Base_c::executeState_EatNow() {}

void daFireBall_Base_c::initializeState_Kill() {
    removeCc();
    mKillTimer = 10;
}

void daFireBall_Base_c::finalizeState_Kill() {}

void daFireBall_Base_c::executeState_Kill() {
    if (--mKillTimer <= 0) {
        deleteRequest();
    }
}

void daFireBall_Base_c::retireHIOnode() {}

void daFireBall_Base_c::entryHIOnode() {}

void daFireBall_Base_c::setEatTongueOff(dActor_c *eatingActor) {
    kill();
}
