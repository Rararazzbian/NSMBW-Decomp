"""Compile draft.cpp, disasm, verify_anon against the full unit range."""
import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'd_a_wm_dance_pakkun.cpp')
OBJ = os.path.join(HERE, 'draft.o')
TXT = os.path.join(HERE, 'draft.txt')

ok, log = H.compile_draft(SRC, OBJ, extra_inc=[os.path.join(HERE, 'shadow_include')], module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-8000:])
    sys.exit(2)
ok2, log2 = H.disasm(OBJ, TXT)
if not ok2:
    print('DISASM FAILED')
    print(log2[-4000:])
    sys.exit(2)

objs = [
    'bin/dtkspl/d_basesNP/obj/auto_00_00161914_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_fn_2_1621C0_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_00_00162290_text.o',
]
cmd = 'python wip/wm_units/verify_anon.py {} 0x161940 0x1622b0 {}'.format(TXT, ' '.join(objs))
os.system(cmd)
