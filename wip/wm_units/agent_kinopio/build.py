import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'd_a_wm_kinopio.cpp')
OBJ = os.path.join(HERE, 'draft.o')
TXT = os.path.join(HERE, 'draft.txt')

ok, log = H.compile_draft(SRC, OBJ, extra_inc=[os.path.join(HERE, 'shadow_include')], module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-10000:])
    sys.exit(2)
ok2, log2 = H.disasm(OBJ, TXT)
if not ok2:
    print('DISASM FAILED')
    print(log2[-4000:])
    sys.exit(2)

objs = [
    'bin/dtkspl/d_basesNP/obj/auto_00_0016C124_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_fn_2_16D1E0_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_00_0016D264_text.o',
]
cmd = 'python wip/wm_units/verify_anon.py {} 0x16c150 0x16d290 {}'.format(TXT, ' '.join(objs))
os.system(cmd)
