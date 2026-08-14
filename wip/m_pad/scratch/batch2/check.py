import sys, os
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'm_pad.cpp')
OBJ = os.path.join(HERE, 'm_pad.o')
TXT = os.path.join(HERE, 'm_pad.txt')
TARGET = os.path.join(ROOT, 'scratch', 'gemini_round8', 'auto_03_8016F330_text.o.txt')
INC = os.path.join(HERE, 'include')

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[INC])
if not ok:
    print("COMPILE FAILED:")
    print(log)
    sys.exit(1)

dok, dlog = harness.disasm(OBJ, TXT)
if not dok:
    print("DISASM FAILED:")
    print(dlog)
    sys.exit(1)

matched, report = harness.diff_fn(TARGET, TXT, 'beginPad__4mPadFv')
print("MATCH!" if matched else "NO MATCH")
print(report)
