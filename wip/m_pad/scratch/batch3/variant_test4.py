import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
MOCK_INC = os.path.join(HERE, 'mock_include')
TARGET_MAIN = os.path.join(ROOT, 'scratch', 'gemini_round8', 'auto_03_8016F330_text.o.txt')

HEADER = '#include <game/mLib/m_pad.hpp>\nnamespace mPad {\nWPADInfo s_WPADInfo[4];\nWPADInfo s_WPADInfoTmp[4];\nbool s_WPADInfoAvailable[4];\nu32 s_GetWPADInfoInterval;\n'
FOOTER = '\n};\n'

variants = {}

variants['L_hybrid_ref_after_first'] = '''
void clearWPADInfo(CH_e ch) {
    s_WPADInfo[ch].dpd = 0;
    WPADInfo &info = s_WPADInfo[ch];
    info.speaker = 0;
    info.attach = 0;
    info.lowBat = 0;
    info.nearempty = 0;
    info.battery = 0;
    info.led = 0;
    info.protocol = 0;
    s_WPADInfoAvailable[ch] = false;
    info.firmware = 0;
}
'''

variants['M_chain_assign'] = '''
void clearWPADInfo(CH_e ch) {
    WPADInfo &info = s_WPADInfo[ch];
    info.dpd = info.speaker = info.attach = info.lowBat = info.nearempty = 0;
    info.battery = info.led = info.protocol = 0;
    s_WPADInfoAvailable[ch] = false;
    info.firmware = 0;
}
'''

variants['N_ref_bound_no_reuse_last'] = '''
void clearWPADInfo(CH_e ch) {
    WPADInfo &info = s_WPADInfo[ch];
    info.dpd = 0;
    info.speaker = 0;
    info.attach = 0;
    info.lowBat = 0;
    info.nearempty = 0;
    info.battery = 0;
    info.led = 0;
    info.protocol = 0;
    s_WPADInfoAvailable[ch] = false;
    s_WPADInfo[ch].firmware = 0;
}
'''

variants['O_two_helper_calls'] = '''
static inline void clearFirstHalf(WPADInfo &info) {
    info.dpd = 0;
    info.speaker = 0;
    info.attach = 0;
    info.lowBat = 0;
    info.nearempty = 0;
    info.battery = 0;
    info.led = 0;
    info.protocol = 0;
}
void clearWPADInfo(CH_e ch) {
    clearFirstHalf(s_WPADInfo[ch]);
    s_WPADInfoAvailable[ch] = false;
    s_WPADInfo[ch].firmware = 0;
}
'''

variants['P_int_offset_manual'] = '''
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
'''

for name, body in variants.items():
    if body.strip().startswith('static inline'):
        src = HEADER + body + FOOTER
    else:
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
