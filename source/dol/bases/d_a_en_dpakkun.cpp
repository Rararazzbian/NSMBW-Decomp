#include <game/bases/d_a_en_dpakkun.hpp>

STATE_VIRTUAL_DEFINE(daEnDpakkun_c, Wait);
STATE_VIRTUAL_DEFINE(daEnDpakkun_c, Appear);
STATE_VIRTUAL_DEFINE(daEnDpakkun_c, Attack);
STATE_VIRTUAL_DEFINE(daEnDpakkun_c, Disappear);

void daEnDpakkun_c::initialize() {
    mTimer = 1;
    mAngle.y = -0x4000;
}

void daEnDpakkun_c::setAttackAnm(float blendFrame) {
    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_attack"), m3d::FORWARD_LOOP);
    mModel.setAnm(mAnm, blendFrame);
    mAnm.setRate(1.0f);
    mAnm.setFrame(0.0f);
}

void daEnDpakkun_c::setVanishAnm() {
    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dead_leaf"), m3d::FORWARD_ONCE);
    mModel.setAnm(mAnm, 1.0f);
}

void daEnDpakkun_c::returnAnm_Ice() {
    setAttackAnm(4.0f);
}

static const float l_move_dir_sign[2] = { 1.0f, -1.0f };

void daEnDpakkun_c::setMoveSpeed(int reverse) {
    static const mVec2_c cs_move_speed[4] = {
        mVec2_c(0.0f, 1.0f),
        mVec2_c(0.0f, -1.0f),
        mVec2_c(1.0f, 0.0f),
        mVec2_c(-1.0f, 0.0f)
    };

    mSpeed.x = l_move_dir_sign[reverse] * cs_move_speed[mPakkunDir].x;
    mSpeed.y = l_move_dir_sign[reverse] * cs_move_speed[mPakkunDir].y;
    mSpeed.z = 0.0f;
}

void daEnDpakkun_c::initializeState_Wait() {
    setAttackAnm(0.0f);
    removeCc();
    mPos = mStartPos;
    mAttacking = 0;
}

void daEnDpakkun_c::finalizeState_Wait() {}

void daEnDpakkun_c::executeState_Wait() {
    if (isPlayerDemo()) {
        mTimer = 72;
    }

    if (--mTimer <= 0 && checkAppear()) {
        reviveCc();
        changeState(StateID_Appear);
    }
}

bool daEnDpakkun_c::checkAppear() {
    return true;
}

void daEnDpakkun_c::initializeState_Appear() {
    mTimer = 32;
    mAngle.y = -0x4000;
    setMoveSpeed(0);
}

void daEnDpakkun_c::finalizeState_Appear() {}

void daEnDpakkun_c::executeState_Appear() {
    mModel.play();
    posMove();

    mTimer--;
    if (mTimer <= 18) {
        mAttacking = 1;
    }

    if (mTimer <= 0) {
        changeState(StateID_Attack);
    } else if (mTimer >= 4) {
        mAngle.y += 0x924;
        if (mTimer == 4) {
            mAngle.y = -0x4000;
        }
    }
}

void daEnDpakkun_c::initializeState_Attack() {
    mTimer = 96;
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
}

void daEnDpakkun_c::finalizeState_Attack() {}

void daEnDpakkun_c::executeState_Attack() {
    mModel.play();

    mAttacking = 1;

    if (--mTimer <= 0) {
        changeState(StateID_Disappear);
    }
}

void daEnDpakkun_c::initializeState_Disappear() {
    mTimer = 32;
    setMoveSpeed(1);
}

void daEnDpakkun_c::finalizeState_Disappear() {}

void daEnDpakkun_c::executeState_Disappear() {
    mModel.play();
    posMove();

    mTimer--;
    if (mTimer <= 18) {
        mAttacking = 0;
    }

    if (mTimer <= 0) {
        mAngle.y = -0x4000;
        mTimer = 72;
        changeState(StateID_Wait);
    } else if (mTimer <= 28) {
        mAngle.y -= 0x924;
    }
}

daEnDpakkun_c::~daEnDpakkun_c() {}
