import sys, os
sys.path.append('tools/auto_decomp')
import harness

test_src = """
#include <game/bases/d_base.hpp>
#include "../scratch/d_pause_manager.hpp"
#include "../scratch/d_s_stage.hpp"
#include "../scratch/d_game_display.hpp"
#include "../scratch/d_stage_timer.hpp"
#include "../scratch/d_quake.hpp"

void test_compilation() {
    if (PauseManager_c::m_instance != 0) {
        PauseManager_c::m_instance->setPauseEnable(true);
        PauseManager_c::m_instance->setPauseEnable(false);
    }
    dGameDisplay_c *disp = dScStage_c::getGameDisplay();
    if (disp != 0) {
        int nums[4] = {1, 2, 3, 4};
        disp->setPlayNum(nums);
        disp->setCoinNum(10);
        disp->setScore(1000);
        disp->setCollect();
    }
    if (dStageTimer_c::m_instance != 0) {
        dStageTimer_c::m_instance->mStopped = true;
    }
    if (dQuake_c::m_instance != 0) {
        if (dQuake_c::m_instance->mFlags & 0x38) {
            if (dQuake_c::m_instance->mFlags & dQuake_c::FLAG_5) {}
            if (dQuake_c::m_instance->mFlags & dQuake_c::FLAG_3) {}
            if (dQuake_c::m_instance->mFlags & dQuake_c::FLAG_4) {}
        }
    }
}
"""

with open('scratch/test_headers.cpp', 'w', newline='\n') as f:
    f.write(test_src)

print('Running compile_draft...')
ok = harness.compile_draft('scratch/test_headers.cpp', 'scratch/test_headers.o')
print('compile_draft result:', ok)
