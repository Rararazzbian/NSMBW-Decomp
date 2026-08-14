#include <game/mLib/m_pad.hpp>
#include <string.h>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void clearWPADInfo(CH_e ch) {
    int i = ch;
    s_WPADInfo[i].dpd = 0;
    s_WPADInfo[i].speaker = 0;
    s_WPADInfo[i].attach = 0;
    s_WPADInfo[i].lowBat = 0;
    s_WPADInfo[i].nearempty = 0;
    s_WPADInfo[i].battery = 0;
    s_WPADInfo[i].led = 0;
    s_WPADInfo[i].protocol = 0;
    s_WPADInfoAvailable[i] = false;
    s_WPADInfo[i].firmware = 0;
}

};
