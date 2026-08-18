
#include <types.h>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_pause_manager.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_game_display.hpp>
#include <game/bases/d_stage_timer.hpp>
#include <game/bases/d_quake.hpp>

void test_update_calls(int mPauseDisable, int mStopTimerInfo, int &mStopTimerInfoOld, int mRest[4], int mScore) {
    if (mPauseDisable == 0) {
        PauseManager_c::m_instance->setPauseEnable(true);
    } else {
        PauseManager_c::m_instance->setPauseEnable(false);
    }

    dGameDisplay_c *disp = dScStage_c::getGameDisplay();
    if (disp != 0) {
        int restCopy[4];
        for (int i = 0; i < 4; i++) restCopy[i] = mRest[i];
        disp->setPlayNum(restCopy);
        disp->setCoinNum(100);
        disp->setScore(mScore);
        disp->setCollect();
    }

    if (mStopTimerInfo != mStopTimerInfoOld) {
        dStageTimer_c::m_instance->mStopped = (mStopTimerInfo != 0);
        mStopTimerInfoOld = mStopTimerInfo;
    }

    if (dQuake_c::m_instance->mFlags & 0x38) {
        if (dQuake_c::m_instance->mFlags & dQuake_c::FLAG_5) {
            // big quake
        } else if (dQuake_c::m_instance->mFlags & dQuake_c::FLAG_3) {
            // small quake unconditional
        } else {
            // small quake timed (FLAG_4)
        }
    }
}
