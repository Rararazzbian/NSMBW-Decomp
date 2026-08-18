import sys, os
sys.path.insert(0, os.path.join('tools','auto_decomp'))
import harness as H

src = 'wip/wm_units/agent_sandpillar/probe/probe.cpp'
obj = 'wip/wm_units/agent_sandpillar/probe/test.o'
txt = 'wip/wm_units/agent_sandpillar/probe/test.txt'

ok, log = H.compile_draft(src, obj, module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-3000:])
    sys.exit(1)
ok, log = H.disasm(obj, txt)
if not ok:
    print('DISASM FAILED')
    print(log[-2000:])
    sys.exit(1)
print('OK')
