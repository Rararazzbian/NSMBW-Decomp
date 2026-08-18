"""One-shot: compile draft.cpp, disasm, verify_anon against createModel only, print result."""
import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

SRC = 'wip/wm_units/agent_ghost/d_a_wm_ghost.cpp'
OBJ = 'wip/wm_units/agent_ghost/draft.o'
TXT = 'wip/wm_units/agent_ghost/draft.txt'

ok, log = H.compile_draft(SRC, OBJ, extra_inc=['wip/wm_units/agent_ghost/shadow_include'], module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-4000:])
    sys.exit(2)
H.disasm(OBJ, TXT)
os.system('python wip/wm_units/verify_anon.py {} 0x163940 0x163a80 bin/dtkspl/d_basesNP/obj/auto_00_00163620_text.o'.format(TXT))
