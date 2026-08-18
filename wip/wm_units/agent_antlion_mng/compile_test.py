import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

ok, log = H.compile_draft('wip/wm_units/agent_antlion_mng/d_a_wm_antlion_mng.cpp',
                           'wip/wm_units/agent_antlion_mng/draft.o',
                           extra_inc=('wip/wm_units/agent_antlion_mng/shadow_include',),
                           module='d_basesNP')
print('compiled:', ok)
print(log)
