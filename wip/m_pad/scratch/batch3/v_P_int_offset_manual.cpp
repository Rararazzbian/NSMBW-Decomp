#include <game/mLib/m_pad.hpp>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void clearWPADInfo(CH_e ch) {
    char *p = (char*)&s_WPADInfo[ch];
    *(u32*)(p + 0x0) = 0;
    *(u32*)(p + 0x4) = 0;
    *(u32*)(p + 0x8) = 0;
    *(u32*)(p + 0xc) = 0;
    *(u32*)(p + 0x10) = 0;
    *(u8*)(p + 0x14) = 0;
    *(u8*)(p + 0x15) = 0;
    *(u8*)(p + 0x16) = 0;
    s_WPADInfoAvailable[ch] = false;
    *(u8*)(p + 0x17) = 0;
}

};
