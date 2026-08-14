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

    // Not this batch's to declare -- setCurrentChannel/getBatteryLevel_ch/the
    // interval accessors belong to other batches. Left out of this proposal;
    // the lead reconciles all of BATCH3's additions against theirs. Note for
    // whoever lands setCurrentChannel: the coordinator's cross-batch codegen
    // check shows it returns CH_e, not void (old g_currentCoreID is loaded
    // into r3 before being overwritten) -- not evidence of ours, passing it on.

    void setWPADInfo(CH_e ch, const WPADInfo &info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    s32 getWPADInfoAsync(CH_e ch);

    extern PadAdditionalData_t g_PadAdditionalData[4];
};
