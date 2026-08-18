import sys, os
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

src = os.path.join(ROOT, 'wip', 'wm_units', 'agent_castle', 'd_a_wm_castle.cpp')
obj = os.path.join(ROOT, 'wip', 'wm_units', 'agent_castle', 'draft.o')
inc = os.path.join(ROOT, 'wip', 'wm_units', 'agent_castle', 'include')

ok, log = H.compile_draft(src, obj, extra_inc=[inc], module='d_basesNP')
print("OK" if ok else "FAIL")
print(log[-4000:])
if ok:
    txt = os.path.join(ROOT, 'wip', 'wm_units', 'agent_castle', 'draft.txt')
    ok2, log2 = H.disasm(obj, txt)
    print("disasm ok" if ok2 else "disasm FAIL")
    print(log2[-2000:])
