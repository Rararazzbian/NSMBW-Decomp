"""Compile draft, disasm, verify_anon against the full unit range."""
import sys, os
ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'd_a_wm_antlion_mng.cpp')
OBJ = os.path.join(HERE, 'draft.o')
TXT = os.path.join(HERE, 'draft.txt')
if len(sys.argv) > 1:
    SRC = os.path.join(HERE, sys.argv[1])
    OBJ = SRC.replace('.cpp', '.o')
    TXT = SRC.replace('.cpp', '.txt')

ok, log = H.compile_draft(SRC, OBJ, extra_inc=[os.path.join(HERE, 'shadow_min')], module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-6000:])
    sys.exit(2)
ok2, log2 = H.disasm(OBJ, TXT)
if not ok2:
    print('DISASM FAILED')
    print(log2[-4000:])
    sys.exit(2)

# CORRECTED WINDOW (r2).  The old 0x15b564-0x15c1d4 window was wrong at BOTH ends:
#   lo 0x15b564 pulled in fn_2_15B570, which is d_a_wm_antlion.cpp's OWN array
#      destructor (that unit's landed .text claim is 0x15ac80-0x15b590), so it is
#      not ours -- and because verify_anon paired it to our arraydtor (which MWCC
#      emits LAST) every other function read as "defined too late".
#   hi 0x15c1d4 cut off fn_2_15C1E0, which IS our array destructor (0x1c bytes,
#      the classic "unit ends after its own arraydtor, past __sinit" shape).
objs = [
    os.path.join(ROOT, 'bin/dtkspl/d_basesNP/obj/auto_00_0015B564_text.o'),
    os.path.join(ROOT, 'bin/dtkspl/d_basesNP/obj/auto_fn_2_15C150_text.o'),
    os.path.join(ROOT, 'bin/dtkspl/d_basesNP/obj/auto_00_0015C1D4_text.o'),
]
cmd = 'python "{}" "{}" 0x15b590 0x15c200 {}'.format(
    os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'), TXT,
    ' '.join('"%s"' % o for o in objs))
os.system(cmd)
