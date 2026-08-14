#include <game/mLib/m_pad.hpp>
#include <string.h>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void clearWPADInfo(CH_e ch) {
    static const WPADInfo zero = {0,0,0,0,0,0,0,0,0};
    s_WPADInfo[ch] = zero;
    s_WPADInfoAvailable[ch] = false;
}

};
