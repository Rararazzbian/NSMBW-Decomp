import sys, os
sys.path.append('tools/auto_decomp')
import harness

update_test_src = """
#include <types.h>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include "../scratch/d_pause_manager.hpp"
#include "../scratch/d_s_stage.hpp"
#include "../scratch/d_game_display.hpp"
#include "../scratch/d_stage_timer.hpp"
#include "../scratch/d_quake.hpp"

void test_update_calls(int mPauseDisable, int mStopTimerInfo, int &mStopTimerInfoOld, int mRest[4], int mScore) {
    // Target 1: PauseManager_c
    if (mPauseDisable == 0) {
        PauseManager_c::m_instance->setPauseEnable(true);
    } else {
        PauseManager_c::m_instance->setPauseEnable(false);
    }

    // Target 2: dScStage_c::getGameDisplay()
    dGameDisplay_c *disp = dScStage_c::getGameDisplay();
    if (disp != 0) {
        int restCopy[4];
        for (int i = 0; i < 4; i++) restCopy[i] = mRest[i];
        // Target 3: 4 dGameDisplay_c methods
        disp->setPlayNum(restCopy);
        disp->setCoinNum(100);
        disp->setScore(mScore);
        disp->setCollect();
    }

    // Target 4: dStageTimer_c mStopped at offset 0xC
    if (mStopTimerInfo != mStopTimerInfoOld) {
        dStageTimer_c::m_instance->mStopped = (mStopTimerInfo != 0);
        mStopTimerInfoOld = mStopTimerInfo;
    }

    // Target 5: dQuake_c FLAGS_e bits
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
"""

with open('scratch/test_update_draft.cpp', 'w', newline='\n') as f:
    f.write(update_test_src)

ok, msg = harness.compile_draft('scratch/test_update_draft.cpp', 'scratch/test_update_draft.o')
print('update test result:', ok, msg)
