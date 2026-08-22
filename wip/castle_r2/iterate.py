import sys, os, subprocess
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

HERE = os.path.join(ROOT, 'wip', 'castle_r2')
src = os.path.join(HERE, 'd_a_wm_castle.cpp')
obj = os.path.join(HERE, 'draft.o')
inc = os.path.join(HERE, 'include')
txt = os.path.join(HERE, 'draft.txt')

if len(sys.argv) > 1:
    src = os.path.join(HERE, sys.argv[1])
    obj = os.path.join(HERE, sys.argv[1].replace('.cpp', '.o'))
    txt = os.path.join(HERE, sys.argv[1].replace('.cpp', '.txt'))

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

r = subprocess.run([sys.executable, os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'),
                     txt, '0x15ecc0', '0x15fbe0',
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_0015ECC0_text.o'),
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_fn_2_15FAE0_text.o'),
                     os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_0015FBB4_text.o')],
                    capture_output=True, text=True)
print(r.stdout)
print(r.stderr)
