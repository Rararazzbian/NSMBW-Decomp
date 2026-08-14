#include <game/mLib/m_pad.hpp>
namespace mPad {
EGG::CoreControllerMgr *g_padMg;
u32 g_currentCoreID;
EGG::CoreController *g_currentCore;
EGG::CoreController *g_core[4];
bool g_IsConnected[4];
u32 g_PadFrame;
PadAdditionalData_t g_PadAdditionalData[4];
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;
u32 s_GetWPADInfoCount;

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

};
