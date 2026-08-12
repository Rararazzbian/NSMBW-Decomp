#include <game/bases/d_a_net_enemy.hpp>
#include <game/bases/d_enemy_manager.hpp>

STATE_VIRTUAL_DEFINE(daNetEnemy_c, NetWait);
STATE_VIRTUAL_DEFINE(daNetEnemy_c, NetMove);

int daNetEnemy_c::execute() {
    if (dEnemyMng_c::m_instance->mWireTurn != 0 && !mNoRespawn) {
        bool inNet = true;
        if (!isState(StateID_NetWait) && !isState(StateID_NetMove)) {
            inNet = false;
        }
        if (!inNet) {
            if (m_524 != 0) {
                changeState(StateID_NetMove);
            } else {
                changeState(StateID_NetWait);
            }
        }
    }

    mStateMgr.executeState();
    calcMdl();
    ActorScrOutCheck(0);
    return SUCCEEDED;
}

void daNetEnemy_c::calcMdl() {}

void daNetEnemy_c::setWireTurn(int turn) {
    dEnemyMng_c::m_instance->mWireTurn = turn;
}

void daNetEnemy_c::initializeState_NetWait() {}
void daNetEnemy_c::finalizeState_NetWait() {}

void daNetEnemy_c::executeState_NetWait() {
    mdlPlay();
    if (dEnemyMng_c::m_instance->mWireTurn == 0) {
        changeState(*mStateMgr.getOldStateID());
    }
}

void daNetEnemy_c::mdlPlay() {}

void daNetEnemy_c::initializeState_NetMove() {
    mCc.mCcData.mCallback = nullptr;
}

void daNetEnemy_c::finalizeState_NetMove() {
    mAmiLayer ^= 1;
    mDirection ^= 1;
    mPos.z = l_Ami_Zpos[mAmiLayer];
    mSpeed.x = -mSpeed.x;
    mCc.mAmiLine = l_Ami_Line[mAmiLayer];
    mCc.mCcData.mCallback = dEn_c::normal_collcheck;
    mBc.mAmiLine = l_Ami_Line[mAmiLayer];
}

void daNetEnemy_c::executeState_NetMove() {
    mdlPlay();
    if (dEnemyMng_c::m_instance->mWireTurn == 0) {
        changeState(*mStateMgr.getOldStateID());
    }
}
