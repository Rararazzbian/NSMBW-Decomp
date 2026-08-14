#include <game/mLib/m_pad.hpp>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void clearWPADInfo(CH_e ch) {
    s_WPADInfo[ch].dpd = 0;
}
extern "C" void getWPADInfoCb(s32 chan, s32 result) {}

void getWPADInfoAsync(CH_e ch) {
    s32 result = WPADGetInfoAsync(ch, &s_WPADInfoTmp[ch], getWPADInfoCb);
    if (result == -1) {
        clearWPADInfo(ch);
    }
}

};
