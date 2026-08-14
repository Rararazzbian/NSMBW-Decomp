import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
MOCK_INC = os.path.join(HERE, 'mock_include')
TARGET_MAIN = os.path.join(ROOT, 'scratch', 'gemini_round8', 'auto_03_8016F330_text.o.txt')

src = '''#include <game/mLib/m_pad.hpp>
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

void create() {
    g_padMg = 0;
}

void setCurrentChannel(CH_e ch) {
    g_currentCoreID = ch;
    g_currentCore = g_core[ch];
}

s32 getBatteryLevel_ch(CH_e ch) {
    if (!s_WPADInfoAvailable[ch]) return -1;
    return s_WPADInfo[ch].battery;
}

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

};
'''
srcfile = os.path.join(HERE, 'v_fullctx.cpp')
objfile = os.path.join(HERE, 'v_fullctx.o')
disfile = os.path.join(HERE, 'v_fullctx_dis.txt')
open(srcfile, 'w').write(src)
ok, log = harness.compile_draft(srcfile, objfile, extra_inc=[MOCK_INC])
if not ok:
    print('COMPILE FAIL'); print(log); sys.exit(1)
ok2, log2 = harness.disasm(objfile, disfile)
matched, report = harness.diff_fn(TARGET_MAIN, disfile, 'clearWPADInfo__4mPadFQ24mPad4CH_e')
print('clearWPADInfo', 'MATCH' if matched else 'DIFFER')
print(report)
