import sys, os
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

HERE = os.path.join(ROOT, 'wip', 'wm_units', 'agent_killer')
src = os.path.join(HERE, 'd_a_wm_killer.cpp')
obj = os.path.join(HERE, 'draft.o')
inc = os.path.join(HERE, 'shadow_include')
txt = os.path.join(HERE, 'draft.txt')

ok, log = H.compile_draft(src, obj, extra_inc=[inc], module='d_basesNP')
if not ok:
    print("BUILD FAIL")
    print(log[-4000:])
    sys.exit(1)

ok2, log2 = H.disasm(obj, txt)
if not ok2:
    print("DISASM FAIL")
    print(log2[-2000:])
    sys.exit(1)

import subprocess
r = subprocess.run([sys.executable, os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'),
                     txt, '0x167940', '0x1686e0',
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_0016793C_text.o'),
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_fn_2_168630_text.o'),
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_001686B4_text.o')],
                    capture_output=True, text=True)
print(r.stdout)
print(r.stderr)
