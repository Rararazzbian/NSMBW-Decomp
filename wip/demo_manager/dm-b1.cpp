// Batch 1/6: lifecycle and core state.
// daPyDemoMng_c: __ct__, __dt__, initStage, initCourseIn, init, update,
// setDemoMode, releaseDemoMode, isDemoMode(x2), deleteNotGoalPlayer,
// calcNotGoalPlayer, setGoalDemoList, isGoalAllEntryPlayer, stopBgmGoalDemo,
// getPoleBelowPlayer.
// See wip/demo_manager/DEMO-MANAGER-SIBMAP.md for the per-function proof
// trail this draft is based on.

#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor.hpp>
#include <game/snd/snd_scene_manager.hpp>

daPyDemoMng_c::daPyDemoMng_c() {
    mspInstance = this;
    init();
}

daPyDemoMng_c::~daPyDemoMng_c() {
    mspInstance = 0;
}

void daPyDemoMng_c::initStage() {
    init();
    mPlayerNo = -1;
}

void daPyDemoMng_c::initCourseIn() {
    m_42 = false;
    m_88 = 0;
    mGoalDemoList[0] = -1;
    mGoalDemoList[1] = -1;
    mGoalDemoList[2] = -1;
    mGoalDemoList[3] = -1;
}

void daPyDemoMng_c::init() {
    mMode = MODE_0;
    m_08 = 0;
    m_0c = 0;
    mFlags = 0;
    mGoalType = 0;
    m_18 = 0;
    mGoalEntryCount = 0;
    mGoalDemoList[0] = -1;
    mGoalDemoList[1] = -1;
    mGoalDemoList[2] = -1;
    mGoalDemoList[3] = -1;
    mGoalCenterPos.x = 0.0f;
    mGoalCenterPos.y = 0.0f;
    mGoalCenterPos.z = 0.0f;
    mNotGoalPlayerTimer = 0;
    m_40 = 0;
    m_41 = 0;
    m_42 = false;
    mBgmStopped = false;
    m_44 = 0;
    m_45 = 0;
    mFireworkPos.x = 0.0f;
    mFireworkPos.y = 0.0f;
    mFireworkPos.z = 0.0f;
    m_54 = 0;
    m_84 = -1;
    m_88 = 0;
    m_58 = 0;
    m_5c = 0;
    mDemoNoQueue[0] = -1;
    mCourseOutList[0] = -1;
    mDemoNoQueue[1] = -1;
    mCourseOutList[1] = -1;
    mDemoNoQueue[2] = -1;
    mCourseOutList[2] = -1;
    mDemoNoQueue[3] = -1;
    mCourseOutList[3] = -1;
    m_8c = -1;
    m_90 = -1;
    m_94 = 0;
}

void daPyDemoMng_c::update() {
    if (m_54 != 0) {
        m_54++;
    }
    if (m_0c != 0) {
        m_0c--;
    }
    switch (mMode) {
    case MODE_1:
        executeGoalDemo();
        break;
    case MODE_2:
        executeGoalCastle();
        break;
    case MODE_4:
        executeStartToride();
        break;
    case MODE_5:
        executeEndToride();
        break;
    }
}

void daPyDemoMng_c::setDemoMode(Mode_e mode, int param) {
    mMode = mode;
    m_08 = param;
}

void daPyDemoMng_c::releaseDemoMode(int param) {
    endControlDemoAll(param);
    mMode = MODE_0;
}

bool daPyDemoMng_c::isDemoMode(Mode_e mode) const {
    return mMode == mode;
}

bool daPyDemoMng_c::isDemoMode(Mode_e mode, int param) const {
    if (isDemoMode(mode) && param == m_08) {
        return true;
    }
    return false;
}

void daPyDemoMng_c::deleteNotGoalPlayer() {
    mNotGoalPlayerTimer = 0x50;
}

void daPyDemoMng_c::calcNotGoalPlayer() {
    if (mNotGoalPlayerTimer != 0 && --mNotGoalPlayerTimer == 0) {
        dActor_c::mExecStopReq &= ~0xf;
        for (int i = 0; i < 4; i++) {
            dAcPy_c *player = daPyMng_c::getCtrlPlayer(i);
            if (player != NULL) {
                player->setHideNotGoalPlayer();
            }
        }
        daYoshi_c *yoshi;
        for (int i = 0; i < 4; i++) {
            yoshi = daPyMng_c::getYoshiDirectP(i);
            if (yoshi != NULL) {
                if (yoshi->getPlrNo() == -1) {
                    yoshi->setHideNotGoalPlayer();
                }
            }
        }
    }
}

int daPyDemoMng_c::setGoalDemoList(int playerNo) {
    for (int i = 0; i < 4; i++) {
        if (mGoalDemoList[i] == -1) {
            mGoalDemoList[i] = playerNo;
            return i;
        }
    }
    return -1;
}

bool daPyDemoMng_c::isGoalAllEntryPlayer() {
    return daPyMng_c::getEntryNum() == (u32)mGoalEntryCount;
}

void daPyDemoMng_c::stopBgmGoalDemo() {
    if (mBgmStopped) {
        return;
    }
    mBgmStopped = true;
    SndSceneMgr::sInstance->fn_8019be60(1);
}

int daPyDemoMng_c::getPoleBelowPlayer(int playerNo) {
    for (int i = 0; i < mGoalEntryCount; i++) {
        if (mGoalDemoList[i] == playerNo && i != 0) {
            return mGoalDemoList[i - 1];
        }
    }
    return -1;
}
