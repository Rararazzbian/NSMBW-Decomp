#include <game/mLib/m_pad.hpp>

namespace mPad {

PadAdditionalData_t::PadAdditionalData_t() { }
PadAdditionalData_t::~PadAdditionalData_t() { }

PadAdditionalData_t g_PadAdditionalData[4];

WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

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

};
