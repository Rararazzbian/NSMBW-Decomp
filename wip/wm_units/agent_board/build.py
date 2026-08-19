"""Compile draft.cpp, disasm, verify_anon against the full unit range."""
import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'd_a_wm_board.cpp')
OBJ = os.path.join(HERE, 'draft.o')
TXT = os.path.join(HERE, 'draft.txt')

ok, log = H.compile_draft(SRC, OBJ, extra_inc=[os.path.join(HERE, 'shadow_include')], module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-6000:])
    sys.exit(2)
ok2, log2 = H.disasm(OBJ, TXT)
if not ok2:
    print('DISASM FAILED')
    print(log2[-4000:])
    sys.exit(2)

objs = [
    'bin/dtkspl/d_basesNP/obj/auto_00_0015C1D4_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_fn_2_15C9C0_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_00_0015CA44_text.o',
]
cmd = 'python wip/wm_units/verify_anon.py {} 0x15c200 0x15ca70 {}'.format(TXT, ' '.join(objs))
os.system(cmd)
