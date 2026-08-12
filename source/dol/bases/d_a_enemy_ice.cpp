#include <game/bases/d_a_enemy_ice.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_allocator_dummy_heap.hpp>

mVec3_c daEnemyIce_c::smc_ICE_DEFSIZE_SQUARE(20.0f, 20.0f, 20.0f);
mVec3_c daEnemyIce_c::smc_ICE_DEFSIZE_TATE(24.0f, 36.0f, 24.0f);
mVec3_c daEnemyIce_c::smc_ICE_DEFSIZE_YOKO(36.0f, 24.0f, 24.0f);
mVec3_c daEnemyIce_c::smc_ICE_DEFSIZE_BIG_SQUARE(64.0f, 64.0f, 64.0f);
mVec3_c daEnemyIce_c::smc_ICE_DEFSIZE_BIG_TATE(64.0f, 96.0f, 64.0f);
mVec3_c daEnemyIce_c::smc_ICE_DEFSIZE_BIG_YOKO(96.0f, 64.0f, 64.0f);

static const mVec3_c l_mdl_defsize[6] = {
    mVec3_c(20.0f, 20.0f, 20.0f),
    mVec3_c(24.0f, 36.0f, 24.0f),
    mVec3_c(36.0f, 24.0f, 24.0f),
    mVec3_c(64.0f, 64.0f, 64.0f),
    mVec3_c(64.0f, 96.0f, 64.0f),
    mVec3_c(96.0f, 64.0f, 64.0f),
};

STATE_DEFINE(daEnemyIce_c, Freeze);
STATE_DEFINE(daEnemyIce_c, Revival);
STATE_DEFINE(daEnemyIce_c, Melt);
STATE_DEFINE(daEnemyIce_c, Break);

ACTOR_PROFILE(ENEMY_ICE, daEnemyIce_c, 2);

int daEnemyIce_c::create() {
    createMdl();

    mAnmTexSrt.setPlayMode(m3d::FORWARD_LOOP, 0);
    mModel.setAnm(mAnmTexSrt, 1.0f);

    if (ACTOR_PARAM(NoMelt)) {
        m_584 = 0;
    } else {
        m_584 = 1;
    }

    calcMdl();
    mStateMgr.changeState(StateID_Freeze);

    return SUCCEEDED;
}

void daEnemyIce_c::createMdl() {
    static const char *cs_ice_mdl_name[6] = {
        "ice_A1", "ice_B1", "ice_C1", "ice_A2", "ice_B2", "ice_C2"
    };
    static const char *cs_ice_anm_name[6] = {
        "ice_A1", "ice_B1", "ice_C1", "ice_A2", "ice_B2", "ice_C2"
    };

    int type = ACTOR_PARAM(IceType);

    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("ice", "g3d/ice.brres");
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(cs_ice_mdl_name[type]);
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1, nullptr);
    dActor_c::setSoftLight_MapObj(mModel);

    mResAnmTexSrt = mResFile.GetResAnmTexSrt(cs_ice_anm_name[type]);
    mAnmTexSrt.create(mdl, mResAnmTexSrt, &mAllocator, nullptr, 1);
    mAnmTexSrt.setAnm(mModel, mResAnmTexSrt, 0, m3d::FORWARD_LOOP);

    mAllocator.adjustFrmHeap();
}

int daEnemyIce_c::execute() {
    if (getConnectParent() == nullptr) {
        deleteRequest();
    } else {
        mStateMgr.executeState();
    }
    return SUCCEEDED;
}

void daEnemyIce_c::postExecute(fBase_c::MAIN_STATE_e status) {
    if (status == SUCCESS) {
        calcMdl();
    }
    dActor_c::postExecute(status);
}

int daEnemyIce_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

void daEnemyIce_c::deleteReady() {}

int daEnemyIce_c::doDelete() {
    mBgCtr.release();
    if (mAllocator.mpHeap != mAllocatorDummyHeap_c::getInstance()) {
        mModel.remove();
        mAnmTexSrt.remove();
    }
    return SUCCEEDED;
}

void daEnemyIce_c::calcMdl() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.YrotM(angle.y);
    mMatrix.XrotM(angle.x);
    mMatrix.ZrotM(angle.z);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);

    calcEffectPos();
}

void daEnemyIce_c::updatePos() {
    dActor_c *parent = (dActor_c *)getConnectParent();
    mPos = parent->mPos + mPosOffset;
}

void daEnemyIce_c::calcMeltScaleRate() {
    // These two locals are dead and generate no code, but the original object's
    // .sdata2 pool holds 0.5f and 0.0f immediately after create()'s 1.0f and
    // before this function's 100.0f, which is only reproducible if both values
    // are created here first. The original presumably had leftover code here.
    // @unofficial
    float unusedHalf = 0.5f;
    float unusedZero = 0.0f;

    float time = *mpMeltTimer;
    if (time == 0.0f) {
        time = 100.0f;
    }

    mMeltScaleRate.x = mScale.x / time;
    mMeltScaleRate.y = mScale.y / time;
    mMeltScaleRate.z = mScale.z / time;
}

void daEnemyIce_c::calcMeltSpeed() {
    float time = *mpMeltTimer;
    if (time == 0.0f) {
        time = 100.0f;
    }

    mSpeed.x = 0.0f;
    mSpeed.y = 0.5f * mIceSize.y / time;
    mSpeed.z = 0.0f;
}

void daEnemyIce_c::calcEffectPos() {
    static const char *cs_jnt_name[6] = {
        "ice_inside", "ice_inside", "ice_inside", "ice_A2", "ice_B2", "ice_C2"
    };

    nw4r::g3d::ResMdl mdl = mModel.getResMdl();
    nw4r::g3d::ResNode node = mdl.GetResNode(cs_jnt_name[ACTOR_PARAM(IceType)]);

    nw4r::math::MTX34 mtx;
    mModel.getNodeWorldMtx(node.GetID(), &mtx);

    mVec3_c pos(mtx._03, mtx._13, 10.0f + (mPos.z + mIceSize.z));
    float ofs = 0.5f * mIceSize.y;

    mEffectPos1 = pos;
    mEffectPos2 = pos;

    if (ACTOR_PARAM(EffectOfs)) {
        mEffectPos1.y = pos.y + ofs;
    }
    mEffectPos2.y = mEffectPos2.y + ofs;
}

void daEnemyIce_c::freezeBeginEffect() {}

void daEnemyIce_c::freezeEffect() {}

void daEnemyIce_c::revivalEffect() {}

void daEnemyIce_c::meltEffect() {}

void daEnemyIce_c::breakEffect() {}

void daEnemyIce_c::initializeState_Freeze() {
    freezeBeginEffect();
}

void daEnemyIce_c::finalizeState_Freeze() {}

void daEnemyIce_c::executeState_Freeze() {
    freezeEffect();
    mAnmTexSrt.play();
    updatePos();
    mBgCtr.calc();
}

void daEnemyIce_c::initializeState_Revival() {
    revivalEffect();
    mBgCtr.release();
}

void daEnemyIce_c::finalizeState_Revival() {}

void daEnemyIce_c::executeState_Revival() {
    deleteRequest();
}

void daEnemyIce_c::initializeState_Melt() {
    meltEffect();
    calcMeltScaleRate();
    calcMeltSpeed();
    mBgCtr.release();
}

void daEnemyIce_c::finalizeState_Melt() {}

void daEnemyIce_c::executeState_Melt() {
    posMove();

    mScale.x -= mMeltScaleRate.x;
    mScale.y -= mMeltScaleRate.y;
    mScale.z -= mMeltScaleRate.z;

    if (*mpMeltTimer <= 0) {
        deleteRequest();
    }
}

void daEnemyIce_c::initializeState_Break() {
    breakEffect();
    mBgCtr.release();
}

void daEnemyIce_c::finalizeState_Break() {}

void daEnemyIce_c::executeState_Break() {
    deleteRequest();
}

daEnemyIce_c::~daEnemyIce_c() {}
