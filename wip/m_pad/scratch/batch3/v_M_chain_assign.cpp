#include <game/mLib/m_pad.hpp>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void clearWPADInfo(CH_e ch) {
    WPADInfo &info = s_WPADInfo[ch];
    info.dpd = info.speaker = info.attach = info.lowBat = info.nearempty = 0;
    info.battery = info.led = info.protocol = 0;
    s_WPADInfoAvailable[ch] = false;
    info.firmware = 0;
}

};
