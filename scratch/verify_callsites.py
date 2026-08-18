import os
import sys
import shutil

sys.path.insert(0, os.path.abspath('.'))
from tools.auto_decomp.harness import compile_draft, disasm

os.makedirs('scratch/mock_include/game/snd', exist_ok=True)
with open('scratch/mock_include/game/snd/snd_audio_mgr.hpp', 'w') as f:
    f.write("""#pragma once
#include <types.h>
#include <lib/nw4r/snd.h>

class SndAudioMgr {
public:
    void startSystemSe(unsigned int soundID, unsigned long);
    void startSystemSe(unsigned long soundID, unsigned long);
    u32 get3DCtrlFlag(unsigned long);
    void setSoundPosition(nw4r::snd::SoundHandle *p, const nw4r::math::VEC2 &pos);

    u8 mPad1[0x100];
    nw4r::snd::SoundArchive *mpSndArc;
    u8 mPad2[0x4b8];
    nw4r::snd::SoundArchivePlayer &mArcPlayer;

public:
    static SndAudioMgr *sInstance;
};
""")

# Test 1: d_pausewindow.cpp
with open('source/dol/bases/d_pausewindow.cpp', 'r') as f:
    pause_src = f.read()
# Replace with (u32)
pause_src_mod = pause_src.replace(
    'SndAudioMgr::sInstance->startSystemSe(SE_SYS_CURSOR, 1);',
    'SndAudioMgr::sInstance->startSystemSe((u32)SE_SYS_CURSOR, 1);'
)
with open('scratch/test_pausewindow.cpp', 'w') as f:
    f.write(pause_src_mod)

ok1, msg1 = compile_draft('scratch/test_pausewindow.cpp', 'scratch/test_pausewindow.o', extra_inc=['scratch/mock_include'])
print("d_pausewindow compile:", ok1)
if not ok1:
    print(msg1)

# Test 2: d_controller_information.cpp
with open('source/d_profileNP/bases/d_controller_information.cpp', 'r') as f:
    ctrl_src = f.read()
ctrl_src_mod = ctrl_src.replace(
    'SndAudioMgr::sInstance->startSystemSe(SE_SYS_BUTTON_SKIP, 1);',
    'SndAudioMgr::sInstance->startSystemSe((u32)SE_SYS_BUTTON_SKIP, 1);'
)
with open('scratch/test_controller_info.cpp', 'w') as f:
    f.write(ctrl_src_mod)

ok2, msg2 = compile_draft('scratch/test_controller_info.cpp', 'scratch/test_controller_info.o', extra_inc=['scratch/mock_include'])
print("d_controller_information compile:", ok2)
if not ok2:
    print(msg2)

# Test 3: d_yes_no_window.cpp
with open('source/d_profileNP/bases/d_yes_no_window.cpp', 'r') as f:
    yn_src = f.read()
yn_src_mod = yn_src.replace(
    'const int SoundEffects[] = { SE_SYS_BACK, SE_SYS_DECIDE, SE_SYS_CURSOR, SE_SYS_DIALOGUE_IN };',
    'const u32 SoundEffects[] = { SE_SYS_BACK, SE_SYS_DECIDE, SE_SYS_CURSOR, SE_SYS_DIALOGUE_IN };'
)
with open('scratch/test_yes_no.cpp', 'w') as f:
    f.write(yn_src_mod)

ok3, msg3 = compile_draft('scratch/test_yes_no.cpp', 'scratch/test_yes_no.o', extra_inc=['scratch/mock_include'])
print("d_yes_no_window compile:", ok3)
if not ok3:
    print(msg3)

# Test 4: d_a_player_base.cpp
with open('source/dol/bases/d_a_player_base.cpp', 'r') as f:
    pl_src = f.read()
pl_src_mod = pl_src.replace(
    'SndAudioMgr::sInstance->startSystemSe(SE_OBJ_GOAL_GET_COIN_BONUS, 1);',
    'SndAudioMgr::sInstance->startSystemSe((u32)SE_OBJ_GOAL_GET_COIN_BONUS, 1);'
)
with open('scratch/test_player_base.cpp', 'w') as f:
    f.write(pl_src_mod)

ok4, msg4 = compile_draft('scratch/test_player_base.cpp', 'scratch/test_player_base.o', extra_inc=['scratch/mock_include'])
print("d_a_player_base compile:", ok4)
if not ok4:
    print(msg4)
