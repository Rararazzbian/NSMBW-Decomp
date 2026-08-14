import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
MOCK_INC = os.path.join(HERE, 'mock_include')
TARGET_MAIN = os.path.join(ROOT, 'scratch', 'gemini_round8', 'auto_03_8016F330_text.o.txt')

HEADER = '#include <game/mLib/m_pad.hpp>\nnamespace mPad {\nWPADInfo s_WPADInfo[4];\nWPADInfo s_WPADInfoTmp[4];\nbool s_WPADInfoAvailable[4];\nu32 s_GetWPADInfoInterval;\n\nvoid clearWPADInfo(CH_e ch) {\n    s_WPADInfo[ch].dpd = 0;\n}\nextern \"C\" void getWPADInfoCb(s32 chan, s32 result) {}\n'
FOOTER = '\n};\n'

variants = {}
variants['void_ret'] = '''
void getWPADInfoAsync(CH_e ch) {
    s32 result = WPADGetInfoAsync(ch, &s_WPADInfoTmp[ch], getWPADInfoCb);
    if (result == -1) {
        clearWPADInfo(ch);
    }
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
        print(name, 'COMPILE FAIL'); print(log); continue
    ok2, log2 = harness.disasm(objfile, disfile)
    matched, report = harness.diff_fn(TARGET_MAIN, disfile, 'getWPADInfoAsync__4mPadFQ24mPad4CH_e')
    print('===', name, 'MATCH' if matched else 'DIFFER', '===')
    print(report)
