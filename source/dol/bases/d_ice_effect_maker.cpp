#include <game/bases/d_ice_effect_maker.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_eff_actor_manager.hpp>

namespace {
dIceEfScale_c l_mdl_scale_tbl[6] = {
    dIceEfScale_c(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f),
    dIceEfScale_c(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.4f, 1.4f, 0.8f),
    dIceEfScale_c(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.3f, 1.3f, 0.8f),
    dIceEfScale_c(1.7f, 1.8f, 1.6f, 1.5f, 2.0f, 2.5f, 2.5f, 1.3f),
    dIceEfScale_c(1.8f, 1.8f, 1.8f, 1.5f, 2.0f, 2.8f, 2.8f, 1.3f),
    dIceEfScale_c(1.8f, 1.8f, 1.7f, 1.8f, 2.0f, 3.0f, 3.0f, 1.3f),
};
}

void dIceEfMaker_c::init(int kind, dIceEfScale_c *scale) {
    mActiveFlags = 0;
    mMode = kind;
    if (scale == nullptr) {
        scale = &l_mdl_scale_tbl[kind];
    }
    setEfScale(*scale);
}

void dIceEfMaker_c::execute() {
    mVec3_c pos = mpActor->getCenterPos();
    pos.z = 5500.0f;

    for (int i = 0; i < 8; i++) {
        if (mActiveFlags & (1 << i)) {
            if (!mpEffects[i]->follow(pos)) {
                mActiveFlags &= ~(1 << i);
            }
        }
    }
}

void dIceEfMaker_c::fin() {}

void dIceEfMaker_c::setEfScale(const dIceEfScale_c &scale) {
    float s0 = scale.mData[0];
    float s1 = scale.mData[1];
    float s2 = scale.mData[2];
    float s3 = scale.mData[3];
    float s4 = scale.mData[4];
    float s5 = scale.mData[5];
    float s6 = scale.mData[6];
    float s7 = scale.mData[7];

    mFreezeEf.mScale.x = s0;
    mFreezeEf.mScale.y = s0;
    mFreezeEf.mScale.z = s0;

    mSmokeEf.mScale.x = s1;
    mSmokeEf.mScale.y = s1;
    mSmokeEf.mScale.z = s1;

    mBreakEf.mScale.x = s2;
    mBreakEf.mScale.y = s2;
    mBreakEf.mScale.z = s2;

    mReleaseEf.mScale.x = s3;
    mReleaseEf.mScale.y = s3;
    mReleaseEf.mScale.z = s3;

    mThawEf.mScale.x = s4;
    mThawEf.mScale.y = s4;
    mThawEf.mScale.z = s4;

    mYoganEf.mScale.x = s5;
    mYoganEf.mScale.y = s5;
    mYoganEf.mScale.z = s5;

    mPoisonEf.mScale.x = s6;
    mPoisonEf.mScale.y = s6;
    mPoisonEf.mScale.z = s6;

    mWaterBreakEf.mScale.x = s7;
    mWaterBreakEf.mScale.y = s7;
    mWaterBreakEf.mScale.z = s7;
}

void dIceEfMaker_c::createEffect(EfKind_e kind) {
    mVec3_c pos = mpActor->getCenterPos();
    pos.z = 5500.0f;

    mpEffects[kind]->create(pos);
    mActiveFlags |= (1 << kind);
}

void dIceEfMaker_c::hahenEffect() {
    mVec3_c pos = mpActor->getCenterPos();
    pos.z = 5500.0f;

    u32 kind = dGameCom::rndInt(4);
    if (mMode >= 3) {
        kind |= 0x10;
    }

    dEffActorMng_c::m_instance->createIceFragEff(pos, kind, -1);
}
