import sys, os
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

src = os.path.join(ROOT, 'wip', 'wm_units', 'agent_castle', 'd_a_wm_castle.cpp')
obj = os.path.join(ROOT, 'wip', 'wm_units', 'agent_castle', 'draft.o')
inc = os.path.join(ROOT, 'wip', 'wm_units', 'agent_castle', 'include')
txt = os.path.join(ROOT, 'wip', 'wm_units', 'agent_castle', 'draft.txt')

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

# Now run verify_anon for the whole range and print the summary table.
import subprocess
r = subprocess.run([sys.executable, os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'),
                     txt, '0x15ecc0', '0x15fbe0',
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_0015ECC0_text.o'),
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_fn_2_15FAE0_text.o'),
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_0015FBB4_text.o')],
                    capture_output=True, text=True)
print(r.stdout)
print(r.stderr)
