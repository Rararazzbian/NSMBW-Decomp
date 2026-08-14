#include <game/mLib/m_pad.hpp>
#include <string.h>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void clearWPADInfo(CH_e ch) {
    WPADInfo *info = &s_WPADInfo[ch];
    memset(info, 0, 0x17);
    s_WPADInfoAvailable[ch] = false;
    info->firmware = 0;
}

};
