import os, sys

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

src = os.path.join(ROOT, 'source', 'dol', 'bases', 'd_multi_manager.cpp')
obj = os.path.join(ROOT, 'scratch', 'gemini_round8', 'd_multi_manager.o')
dis = os.path.join(ROOT, 'scratch', 'gemini_round8', 'd_multi_manager_draft_disasm.txt')
target = os.path.join(ROOT, 'scratch', 'd_multi_manager_disasm.txt')

ok, log = harness.compile_draft(src, obj)
print(f"Compile status: {ok}")
if not ok:
    print(log)
    sys.exit(1)

ok, log = harness.disasm(obj, dis)
print(f"Disasm status: {ok}")
if not ok:
    print(log)
    sys.exit(1)

# List functions from target and draft
target_fns = harness.list_functions(target, with_size=True)
print(f"Target functions ({len(target_fns)}):")
for fn, sz in target_fns:
    matched, report = harness.diff_fn(target, dis, fn)
    print(f"  0x{sz:02X} {fn}: {'MATCH' if matched else 'DIFF'}")
    if not matched:
        print(f"    Report:\n{report}")
