import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
MOCK_INC = os.path.join(HERE, 'mock_include')
TARGET_MAIN = os.path.join(ROOT, 'scratch', 'gemini_round8', 'auto_03_8016F330_text.o.txt')

CLEAR_BODY = '''
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
'''

variants = {}

variants['I_all_globals_full_order'] = '''#include <game/mLib/m_pad.hpp>
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
''' + CLEAR_BODY + '\n};\n'

variants['J_minimal_but_avail_declared_before_wpadinfo'] = '''#include <game/mLib/m_pad.hpp>
namespace mPad {
bool s_WPADInfoAvailable[4];
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
u32 s_GetWPADInfoInterval;
''' + CLEAR_BODY + '\n};\n'

variants['K_setWPADInfo_present_first'] = '''#include <game/mLib/m_pad.hpp>
namespace mPad {
WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void setWPADInfo(CH_e ch, const WPADInfo &info) {
    s_WPADInfo[ch] = info;
    s_WPADInfoAvailable[ch] = true;
}
''' + CLEAR_BODY + '\n};\n'

for name, src in variants.items():
    srcfile = os.path.join(HERE, 'v_%s.cpp' % name)
    objfile = os.path.join(HERE, 'v_%s.o' % name)
    disfile = os.path.join(HERE, 'v_%s_dis.txt' % name)
    open(srcfile, 'w').write(src)
    ok, log = harness.compile_draft(srcfile, objfile, extra_inc=[MOCK_INC])
    if not ok:
        print(name, 'COMPILE FAIL')
        print(log)
        continue
    ok2, log2 = harness.disasm(objfile, disfile)
    matched, report = harness.diff_fn(TARGET_MAIN, disfile, 'clearWPADInfo__4mPadFQ24mPad4CH_e')
    print('===', name, 'MATCH' if matched else 'DIFFER', '===')
    print(report)
    print()
