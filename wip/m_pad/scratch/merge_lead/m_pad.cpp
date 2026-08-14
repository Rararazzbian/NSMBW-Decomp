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

void beginPad() {
    g_PadFrame++;
    g_padMg->beginFrame();

    for (int i = 0; i < 4; i++) {
        PadAdditionalData_t &pad = g_PadAdditionalData[i];
        EGG::CoreController *core = g_padMg->getNthController(i);
        g_core[i] = core;

        if (*((u8 *)core + 0xb1c) & 1) {
            float newX = *(float *)((u8 *)core + 0x6c);
            float newY = *(float *)((u8 *)core + 0x70);
            float dX = newX - pad.mPosX;
            float dY = newY - pad.mPosY;
            pad.mPosX = newX;
            pad.mPosY = newY;
            float ddX = dX - pad.mVelX;
            float ddY = dY - pad.mVelY;
            PadDelta_t delta = { ddX, ddY, dX, dY, newX, newY };
            pad.setAccVel(delta);

            if (!g_IsConnected[i])
                g_IsConnected[i] = true;

            if (s_GetWPADInfoInterval != 0) {
                if (s_GetWPADInfoInterval == 1 || s_GetWPADInfoCount == (u32)i ||
                    (s_GetWPADInfoInterval <= 3 &&
                     (s_GetWPADInfoCount & 1) == ((u32)i & 1))) {
                    getWPADInfoAsync((CH_e)i);
                }
            }
        } else {
            if (g_IsConnected[i]) {
                ((EGG::CoreStatus *)((u8 *)core + 0x18))->init();
                core->sceneReset();
                pad.mPosX = 0.0f;
                pad.mPosY = 0.0f;
                pad.mAccX = 0.0f;
                pad.mAccY = 0.0f;
                pad.mVelX = 0.0f;
                pad.mVelY = 0.0f;
                clearWPADInfo((CH_e)i);
                g_IsConnected[i] = false;
            }
        }
    }

    if (s_GetWPADInfoInterval != 0) {
        if (++s_GetWPADInfoCount > s_GetWPADInfoInterval)
            s_GetWPADInfoCount = 0;
    }

    g_currentCore = g_core[g_currentCoreID];
}

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
