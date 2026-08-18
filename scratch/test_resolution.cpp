
#include <types.h>
#include <constants/sound_list.h>

class SndAudioMgr {
public:
    void startSystemSe(unsigned int soundID, unsigned long);
    void startSystemSe(unsigned long soundID, unsigned long);

    static SndAudioMgr *sInstance;
};

// If SoundEffects is u32:
const u32 SoundEffects[4] = { SE_SYS_BACK, SE_SYS_DECIDE, SE_SYS_CURSOR, SE_SYS_DIALOGUE_IN };

void test_site1_cast() {
    // d_a_player_base.cpp:3967
    SndAudioMgr::sInstance->startSystemSe((u32)SE_OBJ_GOAL_GET_COIN_BONUS, 1);
}

void test_site2_cast() {
    // d_pausewindow.cpp:357
    SndAudioMgr::sInstance->startSystemSe((u32)SE_SYS_CURSOR, 1);
}

void test_site3_cast() {
    // d_controller_information.cpp:87
    SndAudioMgr::sInstance->startSystemSe((u32)SE_SYS_BUTTON_SKIP, 1);
}

void test_site4_5_6_7_array() {
    // d_yes_no_window.cpp:428, 529, 551, 595
    SndAudioMgr::sInstance->startSystemSe(SoundEffects[0], 1);
    SndAudioMgr::sInstance->startSystemSe(SoundEffects[1], 1);
    SndAudioMgr::sInstance->startSystemSe(SoundEffects[2], 1);
    SndAudioMgr::sInstance->startSystemSe(SoundEffects[3], 1);
}
