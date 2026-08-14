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

    /// @unofficial -- offsets 0x0/0x4/0x8/0xc/0x10/0x14 are solid (read directly
    /// out of beginPad's disassembly, which is not in this batch). Member names
    /// are a guess at semantics (looks like cur/prev/delta for a 2-axis value);
    /// the ctor/dtor bodies belong to a different batch and are declared only,
    /// not defined, here.
    struct PadAdditionalData_t {
        PadAdditionalData_t();
        ~PadAdditionalData_t();

        f32 mCurX;   // 0x0
        f32 mCurY;   // 0x4
        f32 mPrevX;  // 0x8
        f32 mPrevY;  // 0xc
        f32 mDeltaX; // 0x10
        f32 mDeltaY; // 0x14
    };
    STATIC_ASSERT(sizeof(PadAdditionalData_t) == 0x18);

    void create();
    void beginPad();
    void endPad();

    /// @returns the previous channel. Confirmed by codegen: the old
    /// g_currentCoreID value is loaded directly into r3 immediately before the
    /// stores, with no other use before blr. A void variant compiles to 7
    /// instructions with no load of the old value; the target is 9. Tested.
    CH_e setCurrentChannel(CH_e ch);

    /// @returns -1 (sentinel) if unavailable, else WPADInfo::battery. Codegen
    /// only proves the return is NOT u8 (li r3,-0x1 loads a full 0xFFFFFFFF;
    /// a u8 return of -1 would load 0xff instead -- tested, it does not match).
    /// s8/s16/s32/u32/u16 all produce byte-identical code for this function;
    /// s32 is chosen as the most idiomatic sentinel-bearing type and is
    /// unproven beyond ruling out u8/the-obvious-wrong-guess.
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
