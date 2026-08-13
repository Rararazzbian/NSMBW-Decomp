// Batch 4/6: goal centre, zorome, control-demo entry.
// daPyDemoMng_c: calcGoalCenterPos, setZoromeGoal, startControlDemoAll,
// isAllPlayerControlDemo, endControlDemoAll, getControlDemoPlayerNum,
// setBossDownPlayerNo, onLandStopReq.
// See wip/demo_manager/DEMO-MANAGER-SIBMAP.md for the per-function proof
// trail this draft is based on.

#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_stage_timer.hpp>
#include <game/mLib/m_vec.hpp>

void daPyDemoMng_c::calcGoalCenterPos() {
    mVec3_c sum(0.0f, 0.0f, 0.0f);
    int count = 0;

    dAcPy_c *player;
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::checkPlayer(i)) {
            player = daPyMng_c::getCtrlPlayer(i);
            if (player != NULL && player->isStatus(daPlBase_c::STATUS_GOAL_POLE_TOUCHED)) {
                count++;
                sum += player->mPos;
            }
        }
    }

    if (count != 0) {
        if (mMode == MODE_1) {
            mGoalCenterPos = sum / (float)count;
        } else {
            mGoalCenterPos.x = sum.x / (float)count;
        }
    }
}

void daPyDemoMng_c::setZoromeGoal() {
    m_40 = 0;
    m_41 = 0;

    if (dInfo_c::mGameFlag & dInfo_c::GAME_FLAG_MULTIPLAYER_MODE)
        return;

    short igt = dStageTimer_c::m_instance->convertToIGT();
    int ones = igt % 10;
    int tens = igt / 10 % 10;

    if (daPyMng_c::isEntryNum1()) {
        if (m_42) {
            m_41 = ones;
            m_40 = 1;
            if (m_41 == 0) {
                m_41 = 1;
            }
        }
    } else {
        if (ones != 0 && ones == tens) {
            m_40 = 1;
            m_41 = ones;
        }
    }
}

bool daPyDemoMng_c::startControlDemoAll() {
    if (daPyMng_c::mNum == 0)
        return false;

    int result = 1;
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::checkPlayer(i)) {
            dAcPy_c *player = daPyMng_c::getCtrlPlayer(i);
            if (player != NULL) {
                if (!player->startControlDemo()) {
                    result = 0;
                }
            }
        }
    }
    return result;
}

bool daPyDemoMng_c::isAllPlayerControlDemo() {
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::checkPlayer(i)) {
            dAcPy_c *player = daPyMng_c::getCtrlPlayer(i);
            if (player != NULL) {
                if (!player->isStatus(daPlBase_c::STATUS_72))
                    return false;
            }
        }
    }
    return true;
}

void daPyDemoMng_c::endControlDemoAll(int param) {
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::checkPlayer(i)) {
            dAcPy_c *player = daPyMng_c::getCtrlPlayer(i);
            if (player != NULL) {
                player->endControlDemo(param);
            }
        }
    }
}

int daPyDemoMng_c::getControlDemoPlayerNum() const {
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::checkPlayer(i)) {
            dAcPy_c *player = daPyMng_c::getCtrlPlayer(i);
            if (player != NULL && player->isDemoType(daPlBase_c::DEMO_PLAYER)) {
                count++;
            }
        }
    }
    return count;
}

void daPyDemoMng_c::setBossDownPlayerNo(int playerNo) {
    m_90 = playerNo;
    mPlayerNo = playerNo;
}

void daPyDemoMng_c::onLandStopReq() {
    dAcPy_c *player;
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::checkPlayer(i)) {
            player = daPyMng_c::getCtrlPlayer(i);
            if (player != NULL) {
                if (!player->isDemoType(daPlBase_c::DEMO_PLAYER)) {
                    player->onStatus(daPlBase_c::STATUS_5F);
                }
            }
        }
    }
}
