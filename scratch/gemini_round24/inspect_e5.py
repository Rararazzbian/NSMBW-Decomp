import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.run_quake_sweep import test_quake_var, make_body
from scratch.gemini_round24.tool import parse_disasm

test_quake_var("E5: (fBase_c*)(mUnk770 >> 31)",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 >> 31) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();"""))

d_all = parse_disasm('scratch/gemini_round24/test_quake_temp.txt')
d_insns = d_all.get('setQuakeDead__18dEnTorideKokoopa_cFv', [])
for idx, (b, t) in enumerate(d_insns):
    print(f'{idx:2d}: {t}')
