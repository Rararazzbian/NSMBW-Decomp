#include <game/bases/d_p_sw_manager.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/snd/snd_audio_mgr.hpp>
#include <game/snd/snd_scene_manager.hpp>

dPSwManager_c *dPSwManager_c::ms_instance;

dPSwManager_c::dPSwManager_c() {
    ms_instance = this;
}

dPSwManager_c::~dPSwManager_c() {
    ms_instance = NULL;
}

void dPSwManager_c::initialize() {
    mData = dBgParameter_c::ms_Instance_p->mPSwData;
}

void dPSwManager_c::execute() {
    ProcMain();
}

void dPSwManager_c::ProcMain() {
    for (int i = 0; i < 3; i++) {
        if (checkSwitch((SwType_e)i)) {
            int timer = getTimer((SwType_e)i);
            if (timer != 0) {
                timer--;
            }
            if ((u32)timer % 60 == 0) {
                if ((u32)timer > 180) {
                    SndAudioMgr::sInstance->startSystemSe(0xAAu, 1ul);
                } else if (timer != 0) {
                    SndAudioMgr::sInstance->startSystemSe(0xABu, 1ul);
                }
            }
            if (timer == 0) {
                SndSceneMgr::sInstance->fn_8019be60(8);
                offSwitch((SwType_e)i);
            }
            setTimer((SwType_e)i, timer);
        }
    }
}

void dPSwManager_c::finalize() {
    dBgParameter_c::ms_Instance_p->mPSwData = mData;
}

u32 dPSwManager_c::checkSwitch(SwType_e type) {
    return mData.mSwitchFlags & (1 << type);
}

bool dPSwManager_c::checkMove() {
    return mData.mSwitchFlags != 0;
}

int dPSwManager_c::getTimer(SwType_e type) {
    return mData.mTimer[type];
}

void dPSwManager_c::onSwitch(SwType_e type, int timer) {
    mData.mSwitchFlags |= (1 << type);
    setTimer(type, timer);
}

void dPSwManager_c::offSwitch(SwType_e type) {
    mData.mSwitchFlags &= ~(1 << type);
}

void dPSwManager_c::setTimer(SwType_e type, int timer) {
    mData.mTimer[type] = timer;
}
