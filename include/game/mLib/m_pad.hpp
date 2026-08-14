#pragma once

#include <lib/egg/core/eggController.h>
#include <lib/revolution/WPAD.h>

namespace mPad {
    enum CH_e {
        MPAD_CH_0,
        MPAD_CH_1,
        MPAD_CH_2,
        MPAD_CH_3
    };

    /// @brief Per-channel pointer state kept alongside the EGG controller.
    /// @unofficial The offsets 0x0 through 0x14 are read directly out of
    /// beginPad's disassembly and the 0x18 size is fixed three ways: the
    /// 0x60-byte g_PadAdditionalData array of four, the existence of
    /// __arraydtor$13953, and the destructor's own size. **The member names are
    /// a reading of the semantics, not evidence** — they look like a
    /// current/previous/delta triple over a two-axis value.
    struct PadAdditionalData_t {
        PadAdditionalData_t();
        ~PadAdditionalData_t();

        f32 mCurX;   ///< [0x00]
        f32 mCurY;   ///< [0x04]
        f32 mPrevX;  ///< [0x08]
        f32 mPrevY;  ///< [0x0C]
        f32 mDeltaX; ///< [0x10]
        f32 mDeltaY; ///< [0x14]
    };

    void create();
    void beginPad();
    void endPad();

    /// @note Returns the PREVIOUS channel, not void. The old g_currentCoreID is
    /// loaded straight into r3 immediately before the stores and nothing else
    /// touches r3 before the blr. A void variant compiles to 7 instructions and
    /// loses that load; the target is 9. CFront cannot encode a return type, so
    /// codegen is the only evidence there is.
    CH_e setCurrentChannel(CH_e ch);

    /// @note `-1` when unavailable, otherwise WPADInfo::battery. The return is
    /// provably NOT `u8`: the target loads a full `0xFFFFFFFF` via `li r3,-0x1`,
    /// where a `u8` return would load `0xFF`. Beyond that the width is
    /// unproven — s8, s16, s32, u16 and u32 all produce byte-identical code
    /// here. @unofficial
    s32 getBatteryLevel_ch(CH_e ch);

    void setWPADInfo(CH_e ch, const WPADInfo &info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    void getWPADInfoAsync(CH_e ch);

    /// @note `unsigned long`, not `u32`. u32 is `unsigned int` here and would
    /// mangle `Ui`; the symbol is `setGetWPADInfoInterval__4mPadFUl`.
    void setGetWPADInfoInterval(ulong interval);
    ulong getGetWPADInfoInterval();

    extern EGG::CoreController *g_currentCore;
    extern CH_e g_currentCoreID;
    extern EGG::CoreController *g_core[4];
};

STATIC_ASSERT(sizeof(mPad::PadAdditionalData_t) == 0x18);
