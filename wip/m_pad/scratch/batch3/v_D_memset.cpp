#include <game/mLib/m_pad.hpp>
#include <string.h>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void clearWPADInfo(CH_e ch) {
    memset(&s_WPADInfo[ch], 0, sizeof(WPADInfo));
    s_WPADInfoAvailable[ch] = false;
}

};
