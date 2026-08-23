#include <game/bases/d_ice_effect_maker.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_eff_actor_manager.hpp>

static dIceEfScale_c l_mdl_scale_tbl[6] = {
    dIceEfScale_c(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f),
    dIceEfScale_c(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.4f, 1.4f, 0.8f),
    dIceEfScale_c(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.3f, 1.3f, 0.8f),
    dIceEfScale_c(1.7f, 1.8f, 1.6f, 1.5f, 2.0f, 2.5f, 2.5f, 1.3f),
    dIceEfScale_c(1.8f, 1.8f, 1.8f, 1.5f, 2.0f, 2.8f, 2.8f, 1.3f),
    dIceEfScale_c(1.8f, 1.8f, 1.7f, 1.8f, 2.0f, 3.0f, 3.0f, 1.3f),
};

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
    dIceEfInf_c **pEf = mpEffects;
    for (int i = 0; i < 8; i++, pEf++) {
        u32 mask = 1 << i;
        if (mActiveFlags & mask) {
            if (!(*pEf)->follow(pos)) {
                mActiveFlags &= ~mask;
            }
        }
    }
}

void dIceEfMaker_c::fin() {}

void dIceEfMaker_c::setEfScale(const dIceEfScale_c &scale) {
    mFreezeEf.mScale.x = scale.mData[0];
    mFreezeEf.mScale.y = scale.mData[0];
    mFreezeEf.mScale.z = scale.mData[0];

    mSmokeEf.mScale.x = scale.mData[1];
    mSmokeEf.mScale.y = scale.mData[1];
    mSmokeEf.mScale.z = scale.mData[1];

    mBreakEf.mScale.x = scale.mData[2];
    mBreakEf.mScale.y = scale.mData[2];
    mBreakEf.mScale.z = scale.mData[2];

    mReleaseEf.mScale.x = scale.mData[3];
    mReleaseEf.mScale.y = scale.mData[3];
    mReleaseEf.mScale.z = scale.mData[3];

    mThawEf.mScale.x = scale.mData[4];
    mThawEf.mScale.y = scale.mData[4];
    mThawEf.mScale.z = scale.mData[4];

    mYoganEf.mScale.x = scale.mData[5];
    mYoganEf.mScale.y = scale.mData[5];
    mYoganEf.mScale.z = scale.mData[5];

    mPoisonEf.mScale.x = scale.mData[6];
    mPoisonEf.mScale.y = scale.mData[6];
    mPoisonEf.mScale.z = scale.mData[6];

    mWaterBreakEf.mScale.x = scale.mData[7];
    mWaterBreakEf.mScale.y = scale.mData[7];
    mWaterBreakEf.mScale.z = scale.mData[7];
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
