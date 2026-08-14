#include <types.h>
#include <game/mLib/m_pad.hpp>

namespace mPad {

// Empty bodies MUST be out of line, not inline in the class: inline gives them
// weak linkage, and retail has both as global. Proven by compile, batch 3.
PadAdditionalData_t::PadAdditionalData_t() { }
PadAdditionalData_t::~PadAdditionalData_t() { }

EGG::CoreController *g_currentCore;
CH_e g_currentCoreID;
EGG::CoreController *g_core[4];

EGG::CoreControllerMgr *g_padMg;
u32 g_PadFrame;
bool g_IsConnected[4];

PadAdditionalData_t g_PadAdditionalData[4];
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
ulong s_GetWPADInfoInterval;
u32 s_GetWPADInfoCount;

// NOTE: no placeholder for the 0x10 at 0x80377F98. It is the
// __register_global_object bookkeeping node (0xC) plus alignment, synthesised
// because g_PadAdditionalData is a static array of objects with destructors.

void create() {
    g_padMg = EGG::CoreControllerMgr::sInstance;
    initWPADInfo();
    beginPad();
    endPad();
}

// beginPad() -- batch 2, not yet merged.

void endPad() {
    g_padMg->endFrame();
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

void setWPADInfo(CH_e ch, const WPADInfo &info) {
    s_WPADInfo[ch] = info;
    s_WPADInfoAvailable[ch] = true;
}

void clearWPADInfo(CH_e ch) {
    s_WPADInfo[ch].dpd = 0;
    s_WPADInfo[ch].speaker = 0;
    s_WPADInfo[ch].attach = 0;
    s_WPADInfo[ch].lowBat = 0;
    s_WPADInfo[ch].nearempty = 0;
    s_WPADInfo[ch].battery = 0;
    s_WPADInfo[ch].led = 0;
    s_WPADInfo[ch].protocol = 0;
    s_WPADInfoAvailable[ch] = false;
    s_WPADInfo[ch].firmware = 0;
}

void initWPADInfo() {
    for (int i = 0; i < 4; i++) {
        clearWPADInfo((CH_e)i);
    }
}

extern "C" void getWPADInfoCb(s32 chan, s32 result) {
    if (s_GetWPADInfoInterval == 0) {
        return;
    }
    switch (result) {
    case 0:
        setWPADInfo((CH_e)chan, s_WPADInfoTmp[chan]);
        break;
    case -1:
        clearWPADInfo((CH_e)chan);
        break;
    }
}

s32 getWPADInfoAsync(CH_e ch) {
    s32 result = WPADGetInfoAsync(ch, &s_WPADInfoTmp[ch], getWPADInfoCb);
    if (result == -1) {
        clearWPADInfo(ch);
    }
    return result;
}

void setGetWPADInfoInterval(ulong interval) {
    s_GetWPADInfoInterval = interval;
    if (interval == 0)
        initWPADInfo();
}

ulong getGetWPADInfoInterval() {
    return s_GetWPADInfoInterval;
}

};
