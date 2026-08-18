import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

ok, log = H.compile_draft('wip/wm_units/agent_antlion/d_a_wm_antlion.cpp',
                           'wip/wm_units/agent_antlion/draft.o', module='d_basesNP')
print('compiled:', ok)
print(log)
