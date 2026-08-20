import sys, os
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

HERE = os.path.join(ROOT, 'wip', 'wm_units', 'agent_lemmy')
src = os.path.join(HERE, 'd_a_lemmy_foothold.cpp')
obj = os.path.join(HERE, 'draft.o')
inc = os.path.join(HERE, 'shadow_include')

ok, log = H.compile_draft(src, obj, extra_inc=[inc], module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-10000:])
    sys.exit(2)

txt = os.path.join(HERE, 'draft.txt')
ok2, log2 = H.disasm(obj, txt)
if not ok2:
    print('DISASM FAILED')
    print(log2[-4000:])
    sys.exit(2)

objs = [
    'bin/dtkspl/d_basesNP/obj/auto_00_000C58E0_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_00_000C6D50_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_fn_2_C6920_text.o',
]
cmd = 'python wip/wm_units/verify_anon.py {} 0xc5c90 0xc7270 {}'.format(txt, ' '.join(objs))
os.system(cmd)
