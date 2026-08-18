import os, sys, shutil

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

mock_dir = os.path.join(ROOT, 'scratch', 'gemini_round8', 'mock_include')
os.makedirs(os.path.join(mock_dir, 'game', 'mLib'), exist_ok=True)

# Copy m_vec.hpp and remove ~mVec2_c() {}
with open(os.path.join(ROOT, 'include', 'game', 'mLib', 'm_vec.hpp'), 'r') as f:
    content = f.read()

# Replace ~mVec2_c() {} with nothing
content_mod = content.replace('~mVec2_c() {}', '')
# Also check if EGG::Vector2f has destructor in eggVector.h
with open(os.path.join(mock_dir, 'game', 'mLib', 'm_vec.hpp'), 'w') as f:
    f.write(content_mod)

src = os.path.join(ROOT, 'source', 'dol', 'bases', 'd_multi_manager.cpp')
obj = os.path.join(ROOT, 'scratch', 'gemini_round8', 'd_multi_manager_clean.o')
dis = os.path.join(ROOT, 'scratch', 'gemini_round8', 'd_multi_manager_clean_disasm.txt')
target = os.path.join(ROOT, 'scratch', 'd_multi_manager_disasm.txt')

ok, log = harness.compile_draft(src, obj, extra_inc=[mock_dir])
print(f'Compile clean: {ok}')
if not ok:
    print(log)
    sys.exit(1)

ok, log = harness.disasm(obj, dis)
print(f'Disasm clean: {ok}')

fns = harness.list_functions(target, with_size=True)
all_match = True
for fn, sz in fns:
    matched, report = harness.diff_fn(target, dis, fn)
    status = "MATCH" if matched else "DIFF"
    print(f'  0x{sz:02X} {fn}: {status}')
    if not matched:
        all_match = False
        print(f'    {report}')

draft_fns = harness.list_functions(dis, with_size=True)
print(f'Target fn count: {len(fns)}, Draft fn count: {len(draft_fns)}')
for fn, sz in draft_fns:
    print(f'    Emitted: 0x{sz:02X} {fn}')
