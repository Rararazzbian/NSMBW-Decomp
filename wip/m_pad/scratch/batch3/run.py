import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'm_pad.cpp')
OBJ = os.path.join(HERE, 'm_pad.o')
DIS = os.path.join(HERE, 'm_pad_disasm.txt')
MOCK_INC = os.path.join(HERE, 'mock_include')

TARGET_MAIN = os.path.join(ROOT, 'scratch', 'gemini_round8', 'auto_03_8016F330_text.o.txt')
TARGET_TAIL = os.path.join(ROOT, 'scratch', 'gemini_round8', 'auto_03_8016F808_text.o.txt')
TARGET_SINIT = os.path.join(HERE, '..', '..', 'scratch', 'batch3', 'sinit_target.txt')
TARGET_SINIT = os.path.join(HERE, 'sinit_target.txt')

FUNCS = [
    ('setWPADInfo__4mPadFQ24mPad4CH_eRC8WPADInfo', TARGET_MAIN, 0x68),
    ('clearWPADInfo__4mPadFQ24mPad4CH_e', TARGET_MAIN, 0x44),
    ('initWPADInfo__4mPadFv', TARGET_MAIN, 0x3C),
    ('getWPADInfoCb', TARGET_MAIN, 0x3C),
    ('getWPADInfoAsync__4mPadFQ24mPad4CH_e', TARGET_MAIN, 0x64),
    ('__sinit_\\m_pad_cpp', TARGET_SINIT, 0x58),
    ('__ct__Q24mPad19PadAdditionalData_tFv', TARGET_TAIL, 0x4),
    ('__dt__Q24mPad19PadAdditionalData_tFv', TARGET_TAIL, 0x40),
    ('__arraydtor$13953', TARGET_TAIL, 0x1C),
]

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[MOCK_INC])
print('COMPILE:', 'OK' if ok else 'FAIL')
if not ok:
    print(log)
    sys.exit(1)

ok2, log2 = harness.disasm(OBJ, DIS)
print('DISASM:', 'OK' if ok2 else 'FAIL')
if not ok2:
    print(log2)
    sys.exit(1)

all_match = True
for name, target, expect_size in FUNCS:
    matched, report = harness.diff_fn(target, DIS, name)
    status = 'MATCH' if matched else 'DIFFER'
    print('--- %s [%s] ---' % (name, status))
    print(report)
    if not matched:
        all_match = False

print()
print('ALL MATCH' if all_match else 'SOME MISMATCHES')
