import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'gemini_icefx')
SHADOW = os.path.join(BASE, 'shadow')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness
import fndiff
import poolcheck

TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', 'dol_bases_d_ice_effect_maker', 'target.txt')
SRC = os.path.join(BASE, 'd_ice_effect_maker.cpp')
OBJ = os.path.join(BASE, 'd_ice_effect_maker.o')
TXT = os.path.join(BASE, 'd_ice_effect_maker.txt')

def build_and_diff():
    inc = (SHADOW,) if os.path.isdir(SHADOW) else ()
    ok, log = harness.compile_draft(SRC, OBJ, extra_inc=inc, module='wiimj2d')
    if not ok:
        print('COMPILE ERROR:\n' + log)
        return False
    ok, log = harness.disasm(OBJ, TXT)
    if not ok:
        print('DISASM ERROR:\n' + log)
        return False

    print('=== FNDIFF ALL ===')
    res = fndiff.main_diff(TARGET, TXT) if hasattr(fndiff, 'main_diff') else None
    return True

if __name__ == '__main__':
    build_and_diff()
