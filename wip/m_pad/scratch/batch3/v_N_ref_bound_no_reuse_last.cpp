#include <game/mLib/m_pad.hpp>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void clearWPADInfo(CH_e ch) {
    WPADInfo &info = s_WPADInfo[ch];
    info.dpd = 0;
    info.speaker = 0;
    info.attach = 0;
    info.lowBat = 0;
    info.nearempty = 0;
    info.battery = 0;
    info.led = 0;
    info.protocol = 0;
    s_WPADInfoAvailable[ch] = false;
    s_WPADInfo[ch].firmware = 0;
}

};
