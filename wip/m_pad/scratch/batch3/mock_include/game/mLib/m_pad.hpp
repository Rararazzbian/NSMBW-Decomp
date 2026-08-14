#pragma once

#include <types.h>
#include <lib/egg/core/eggController.h>
#include <lib/revolution/WPAD/WPAD.h>

namespace mPad {
    enum CH_e {
        MPAD_CH_0,
        MPAD_CH_1,
        MPAD_CH_2,
        MPAD_CH_3
    };

    /// @unofficial Layout proven by mPad::beginPad (not ours -- see BATCH3.md).
    /// Six floats, sizeof == 0x18, matching g_PadAdditionalData__4mPad's
    /// array stride. Names are guesses; only the float-ness, count and order
    /// are evidenced.
    struct PadAdditionalData_t {
        PadAdditionalData_t();
        ~PadAdditionalData_t();

        f32 posX;   ///< @unofficial CoreController+0x6c
        f32 posY;   ///< @unofficial CoreController+0x70
        f32 velX;   ///< @unofficial posX - previous posX
        f32 velY;   ///< @unofficial posY - previous posY
        f32 accX;   ///< @unofficial velX - previous velX
        f32 accY;   ///< @unofficial velY - previous velY
    };

    void create();
    void beginPad();
    void endPad();
    void setCurrentChannel(CH_e ch);
    s32 getBatteryLevel_ch(CH_e ch);
    void setWPADInfo(CH_e ch, const WPADInfo &info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    s32 getWPADInfoAsync(CH_e ch);
    void setGetWPADInfoInterval(u32 interval);
    u32 getGetWPADInfoInterval();

    extern EGG::CoreController *g_currentCore;
    extern u32 g_currentCoreID;
    extern EGG::CoreController *g_core[4];
    extern PadAdditionalData_t g_PadAdditionalData[4];
};
