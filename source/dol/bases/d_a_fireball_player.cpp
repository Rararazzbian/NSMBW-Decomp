#include <game/bases/d_a_fireball_player.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_cc.hpp>
#include <game/bases/d_ef.hpp>
#include <game/bases/d_eff_actor_manager.hpp>
#include <game/bases/d_enemy.hpp>
#include <game/framework/f_profile.hpp>
#include <game/framework/f_profile_name.hpp>
#include <constants/sound_list.h>

ACTOR_PROFILE(PL_FIREBALL, daFireBall_Player_c, 2);

int daFireBall_Player_c::sm_FireBallCnt[4];
int daFireBall_Player_c::sm_AliveFireBallCnt[4];

const sCcDatNewF l_fball_cc_data = {
    0.0f, 0.0f,
    3.0f, 3.0f,
    CC_KIND_TAMA,
    CC_ATTACK_FIREBALL,
    BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_ENEMY) |
      BIT_FLAG(CC_KIND_BALLOON) | BIT_FLAG(CC_KIND_ITEM) | BIT_FLAG(CC_KIND_TAMA) | BIT_FLAG(CC_KIND_KILLER),
    BIT_FLAG(CC_ATTACK_KOOPA_FIRE) | BIT_FLAG(CC_ATTACK_YOSHI_EAT),
    CC_STATUS_NONE,
    daFireBall_Player_c::fireball_collcheck
};

STATE_VIRTUAL_DEFINE(daFireBall_Player_c, Move);

void daFireBall_Player_c::entryHIOnode() {}

void daFireBall_Player_c::retireHIOnode() {}

int daFireBall_Player_c::createCheck() {
    bool isCreateOK = CheckFireBallLimit(ACTOR_PARAM(PlayerNo), ACTOR_PARAM(LimitMode));

    mPlayerNo = ACTOR_PARAM(PlayerNo);
    mPlayerNum = ACTOR_PARAM(PlayerNo);

    sm_FireBallCnt[ACTOR_PARAM(PlayerNo)]++;
    sm_AliveFireBallCnt[ACTOR_PARAM(PlayerNo)]++;

    mAliveTimer = 60;

    return isCreateOK;
}

void daFireBall_Player_c::chgZpos() {
    if (mLayer == LAYER_1) {
        if (dBc_c::checkWireNet(mPos.x, mPos.y, mLayer)) {
            if (mStartPos.z >= 0.0f) {
                mPos.z = 2000.0f;
            } else {
                mPos.z = mStartPos.z;
            }
        } else {
            mPos.z = 2000.0f;
        }
    } else if (mLayer == LAYER_2) {
        mPos.z = -1800.0f;
    }
}

int daFireBall_Player_c::initialize() {
    static const float cs_speed_x[] = {3.625f, -3.625f};

    mLayer = ACTOR_PARAM(Layer);
    mCc.mLayer = ACTOR_PARAM(Layer);
    mBc.mLayer = ACTOR_PARAM(Layer);

    mCc.mAmiLine = ACTOR_PARAM(AmiLine);
    mBc.mAmiLine = ACTOR_PARAM(AmiLine);
    mRc.mLineKind = ACTOR_PARAM(AmiLine);

    mDirection = ACTOR_PARAM(Direction);

    float groundHeight = 0.0f;
    if (checkInitLine(groundHeight)) {
        mPos.y = groundHeight + 4.0f;
    }

    if (checkInitVanish()) {
        mVec3_c effPos(mPos.x, mPos.y, 5500.0f);
        dEf::createEffect_change("Wm_mr_fireball_hit", 0, &effPos, nullptr, nullptr);

        dAudio::SoundEffectID_t(SE_OBJ_FIREBALL_DISAPP).playMapSound(mPos, 0);
        return CANCELED;
    }

    mAccelY = smc_GRAVITY;
    mMaxFallSpeed = smc_MAXFALLSPEED;
    mSpeed.set(cs_speed_x[mDirection], -3.0f, 0.0f);

    return SUCCEEDED;
}

void daFireBall_Player_c::setCc() {
    mCc.set(this, (sCcDatNewF *) &l_fball_cc_data);
    mCc.entry();
}

bool daFireBall_Player_c::checkInitVanish() {
    if (mBc.checkWall(nullptr)) {
        return true;
    }

    dAcPy_c *player = daPyMng_c::getPlayer(mPlayerNo);
    mVec3_c a(player->mPos.x, mPos.y, mPos.z);
    mVec3_c b = mPos;
    float c = 0.0f;

    return dBc_c::checkWall(&a, &b, &c, mLayer, 1, nullptr);
}

bool daFireBall_Player_c::checkInitLine(float &groundHeight) {
    mVec3_c pos(mPos.x, mPos.y + 10.0f, mPos.z);

    float height = 0.0f;
    if (dBc_c::checkGround(&pos, &height, mLayer, 1, -1)) {
        if (height < pos.y && height >= mPos.y - 3.0f) {
            groundHeight = height;
            return true;
        }
    }

    return false;
}

void daFireBall_Player_c::beginSplash(float height) {
    dAudio::SoundEffectID_t(SE_OBJ_FIREBALL_SPLASH).playMapSound(mPos, 0);

    mVec3_c pos(mPos.x, height, 5500.0f);

    u32 splashFlags;
    if (pos.y == mPos.y) {
        splashFlags = mLayer << 16 | 1;
    } else {
        splashFlags = mLayer << 16 | 2;
    }

    dEffActorMng_c::m_instance->createWaterSplashEff(pos, splashFlags, -1, mVec3_c(1.0f, 1.0f, 1.0f));
    dBg_c::m_bg_p->setWaterInWave(mPos.x, mPos.y, 8);
}

void daFireBall_Player_c::fireball_collcheck(dCc_c *self, dCc_c *other) {
    if ((other->mCcData.mVsDamage & (1 << CC_ATTACK_FIREBALL)) == 0) {
        return;
    }

    dActor_c *otherActor = other->getOwner();
    daFireBall_Player_c *thisFireball = (daFireBall_Player_c *) self->getOwner();

    self->mInfo |= CC_NO_HIT;

    if (otherActor->mKind == dActor_c::STAGE_ACTOR_ENEMY) {
        if (otherActor->mProfName == fProfile::EN_MARUTA) {
            if (thisFireball->mSpeed.y < 0.0f) {
                thisFireball->mSpeed.y = 4.0f;
            }
        } else {
            ((dEn_c *) otherActor)->boyonBegin();
            thisFireball->kill();
        }
    } else if (otherActor->mKind != dActor_c::STAGE_ACTOR_PLAYER) {
        if (other->mCcData.mAttack == CC_ATTACK_KOOPA_FIRE) {
            dAudio::SoundEffectID_t(SE_OBJ_EMY_FIRE_DISAPP).playMapSound(thisFireball->mPos, 0);
            thisFireball->kill();
        } else if (other->mCcData.mKind == CC_KIND_ENEMY) {
            thisFireball->kill();
        }
    }
}

int daFireBall_Player_c::doDelete() {
    if (--sm_FireBallCnt[mPlayerNum] < 0) {
        sm_FireBallCnt[mPlayerNum] = 0;
    }

    if (mAliveTimer != 0) {
        if (--sm_AliveFireBallCnt[mPlayerNum] < 0) {
            sm_AliveFireBallCnt[mPlayerNum] = 0;
        }
    }

    return daFireBall_Base_c::doDelete();
}

void daFireBall_Player_c::initializeState_Move() {
    daFireBall_Base_c::initializeState_Move();
}

void daFireBall_Player_c::finalizeState_Move() {
    daFireBall_Base_c::finalizeState_Move();
}

void daFireBall_Player_c::executeState_Move() {
    if (mAliveTimer != 0 && --mAliveTimer == 0) {
        if (--sm_AliveFireBallCnt[mPlayerNum] < 0) {
            sm_AliveFireBallCnt[mPlayerNum] = 0;
        }
    }

    calcFallSpeed();
    posMove();

    if (mRc.check(mBc.checkFoot(), 0, 0)) {
        mBc.mFlags |= dBc_c::FLAG_15;
    }

    if (mBc.checkHead(0)) {
        dAudio::SoundEffectID_t(SE_OBJ_FIREBALL_DISAPP).playMapSound(mPos, 0);
        kill();
        return;
    }

    if (mBc.checkWall(nullptr)) {
        dAudio::SoundEffectID_t(SE_OBJ_FIREBALL_DISAPP).playMapSound(mPos, 0);
        kill();
        return;
    }

    if (killcheck_Bg() || killcheck_Ride()) {
        dAudio::SoundEffectID_t(SE_OBJ_FIREBALL_DISAPP).playMapSound(mPos, 0);
        kill();
        return;
    }

    if (boundCheck()) {
        setSakaSpeed(mBc.getSakaType(), mBc.getSakaDir());
    }
}

bool daFireBall_Player_c::killcheck_Bg() {
    if ((mBc.mFlags & dBc_c::FLAG_15) && (mRc.isRideFlag(0x200) & 0xFFFF)) {
        return true;
    }

    if (mBc.getFootAttr() == 3) {
        return true;
    }

    return mBc.getSakaAngle(mDirection) >= 0x2D16;
}

bool daFireBall_Player_c::killcheck_Ride() {
    if (mRc.getRide() && (mRc.getRide()->mFlags & 0x1000)) {
        return true;
    }

    return false;
}

u32 daFireBall_Player_c::boundCheck() {
    return mBc.mFlags & dBc_c::FLAG_FOOT;
}

void daFireBall_Player_c::setSakaSpeed(u8 sakaType, u8 sakaDir) {
    static const float cs_saka_speed_x[5][2] = {
        {3.625f, 3.625f},
        {3.5f, 3.625f},
        {3.5f, 3.625f},
        {3.1875f, 3.625f},
        {3.0625f, 3.625f}
    };
    static const float cs_saka_speed_y[5][2] = {
        {4.0f, 4.0f},
        {5.75f, 3.0f},
        {5.75f, 3.0f},
        {7.5f, 2.0f},
        {10.25f, 1.0f}
    };

    u8 idx = 0;
    if (mDirection == sakaDir) {
        idx = 1;
    }

    mSpeed.set(l_EnMuki[mDirection] * cs_saka_speed_x[sakaType][idx], cs_saka_speed_y[sakaType][idx], 0.0f);
}

bool daFireBall_Player_c::CheckFireBallLimit(int playerNo, int limitMode) {
    if (daFireBall_Player_c::sm_FireBallCnt[playerNo] < smc_MAX_FIREBALL_COUNT) {
        if (limitMode == 1) {
            return true;
        }

        if (daFireBall_Player_c::sm_AliveFireBallCnt[playerNo] < smc_MAX_ALIVE_FIREBALL_COUNT) {
            return true;
        }
    }

    return false;
}
