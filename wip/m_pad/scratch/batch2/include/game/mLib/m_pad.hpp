#pragma once

#include <lib/egg/core/eggController.h>

namespace mPad {
    enum CH_e {
        MPAD_CH_0,
        MPAD_CH_1,
        MPAD_CH_2,
        MPAD_CH_3
    };

    struct PadAdditionalData_t {
        float mPosX;   // 0x0
        float mPosY;   // 0x4
        float mVelX;   // 0x8
        float mVelY;   // 0xc
        float mAccX;   // 0x10
        float mAccY;   // 0x14
    }; // size 0x18

    void create();
    void beginPad();
    void endPad();
    void setCurrentChannel(CH_e ch);
    s32 getBatteryLevel_ch(CH_e ch);
    void setWPADInfo(CH_e ch, const WPADInfo &info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    void getWPADInfoAsync(CH_e ch);
    void setGetWPADInfoInterval(u32 interval);
    u32 getGetWPADInfoInterval();

    extern EGG::CoreControllerMgr *g_padMg;
    extern int g_currentCoreID;
    extern EGG::CoreController *g_currentCore;
    extern bool g_IsConnected[4];
    extern u32 g_PadFrame;
    extern u32 s_GetWPADInfoInterval;
    extern u32 s_GetWPADInfoCount;

    extern EGG::CoreController *g_core[4];
    extern PadAdditionalData_t g_PadAdditionalData[4];
};
