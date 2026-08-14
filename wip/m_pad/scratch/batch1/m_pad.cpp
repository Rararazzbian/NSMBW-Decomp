#include <types.h>
#include <game/mLib/m_pad.hpp>

namespace mPad {
    // ---- public API globals (declared in m_pad.hpp) ------------------------
    EGG::CoreController *g_currentCore;
    CH_e g_currentCoreID;
    EGG::CoreController *g_core[4];

    // ---- file-private bookkeeping (not exposed via the header) -------------
    // No evidence any other TU references these; only functions inside this
    // TU touch them in the disassembly seen so far. @unofficial linkage call.
    EGG::CoreControllerMgr *g_padMg;
    u32 g_PadFrame;
    bool g_IsConnected[4];
    bool s_WPADInfoAvailable[4];
    ulong s_GetWPADInfoInterval;
    u32 s_GetWPADInfoCount;
    WPADInfo s_WPADInfo[4];
    WPADInfo s_WPADInfoTmp[4];
    PadAdditionalData_t g_PadAdditionalData[4];

    // Unclaimed 0x10 gap in .bss between g_core (ends 0x80377F98) and
    // g_PadAdditionalData (starts 0x80377FA8). No symbol in the map, and none
    // of the six functions in this batch reference it. @unofficial placeholder
    // only -- shape unknown, only the total size (0x10) is evidenced.
    u8 pad_80377F98[0x10];

    // ---- this batch's six functions -----------------------------------------

    void endPad() {
        g_padMg->endFrame();
    }

    void create() {
        g_padMg = EGG::CoreControllerMgr::sInstance;
        initWPADInfo();
        beginPad();
        endPad();
    }

    CH_e setCurrentChannel(CH_e ch) {
        CH_e old = g_currentCoreID;
        g_currentCoreID = ch;
        g_currentCore = g_core[ch];
        return old;
    }

    s32 getBatteryLevel_ch(CH_e ch) {
        if (!s_WPADInfoAvailable[ch])
            return -1;
        return s_WPADInfo[ch].battery;
    }

    void setGetWPADInfoInterval(ulong interval) {
        s_GetWPADInfoInterval = interval;
        if (interval == 0)
            initWPADInfo();
    }

    ulong getGetWPADInfoInterval() {
        return s_GetWPADInfoInterval;
    }
}
