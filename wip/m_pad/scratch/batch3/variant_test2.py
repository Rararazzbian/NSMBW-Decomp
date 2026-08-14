import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
MOCK_INC = os.path.join(HERE, 'mock_include')
TARGET_MAIN = os.path.join(ROOT, 'scratch', 'gemini_round8', 'auto_03_8016F330_text.o.txt')

HEADER = '#include <game/mLib/m_pad.hpp>\n#include <string.h>\nnamespace mPad {\nWPADInfo s_WPADInfo[4];\nWPADInfo s_WPADInfoTmp[4];\nbool s_WPADInfoAvailable[4];\nu32 s_GetWPADInfoInterval;\n'
FOOTER = '\n};\n'

variants = {}

variants['D_memset'] = '''
void clearWPADInfo(CH_e ch) {
    memset(&s_WPADInfo[ch], 0, sizeof(WPADInfo));
    s_WPADInfoAvailable[ch] = false;
}
'''

variants['E_memset_after_avail_before_last'] = '''
void clearWPADInfo(CH_e ch) {
    WPADInfo *info = &s_WPADInfo[ch];
    memset(info, 0, 0x17);
    s_WPADInfoAvailable[ch] = false;
    info->firmware = 0;
}
'''

variants['F_index_var'] = '''
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
'''

variants['G_zero_struct_assign'] = '''
void clearWPADInfo(CH_e ch) {
    static const WPADInfo zero = {0,0,0,0,0,0,0,0,0};
    s_WPADInfo[ch] = zero;
    s_WPADInfoAvailable[ch] = false;
}
'''

variants['H_avail_before_last_field_direct_no_ref'] = '''
void clearWPADInfo(CH_e ch) {
    s_WPADInfo[ch].dpd = 0;
    s_WPADInfo[ch].speaker = 0;
    s_WPADInfo[ch].attach = 0;
    s_WPADInfo[ch].lowBat = 0;
    s_WPADInfo[ch].nearempty = 0;
    s_WPADInfo[ch].battery = 0;
    s_WPADInfo[ch].led = 0;
    s_WPADInfo[ch].protocol = 0;
    s_WPADInfo[ch].firmware = 0;
    s_WPADInfoAvailable[ch] = false;
}
'''

for name, body in variants.items():
    src = HEADER + body + FOOTER
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
