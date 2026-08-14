#pragma once

#include <types.h>
#include <revolution/WPAD/WPAD.h>
#include <lib/egg/core/eggController.h>

namespace mPad {
    enum CH_e {
        MPAD_CH_0,
        MPAD_CH_1,
        MPAD_CH_2,
        MPAD_CH_3
    };

    // Batch1 proposed mCurX/mCurY/mPrevX/mPrevY/mDeltaX/mDeltaY from offsets
    // alone (beginPad wasn't in their scope). beginPad's actual data flow
    // shows offsets 0x8/0xc are NOT a raw "previous" sample and 0x10/0x14 are
    // NOT a plain "delta" -- see BATCH2.md for the derivation. Renamed here
    // to match the derived semantics (position / velocity / acceleration);
    // offsets 0x0/0x4/0x8/0xc/0x10/0x14 are solid either way.
    struct PadAdditionalData_t {
        PadAdditionalData_t();
        ~PadAdditionalData_t();

        f32 mPosX; // 0x0
        f32 mPosY; // 0x4
        f32 mVelX; // 0x8
        f32 mVelY; // 0xc
        f32 mAccX; // 0x10
        f32 mAccY; // 0x14
    };
    STATIC_ASSERT(sizeof(PadAdditionalData_t) == 0x18);

    void create();
    void beginPad();
    void endPad();
    CH_e setCurrentChannel(CH_e ch);
    s32 getBatteryLevel_ch(CH_e ch);
    void setWPADInfo(CH_e ch, const WPADInfo &info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    void getWPADInfoAsync(CH_e ch);
    void setGetWPADInfoInterval(ulong interval);
    ulong getGetWPADInfoInterval();

    extern EGG::CoreController *g_currentCore;
    extern CH_e g_currentCoreID;
    extern EGG::CoreController *g_core[4];
};
