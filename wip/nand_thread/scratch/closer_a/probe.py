import os, sys, re
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

WORK = os.path.join(ROOT, 'wip', 'nand_thread', 'scratch', 'closer_a')

def try_variant(label, src_text, fn_pattern):
    src = os.path.join(WORK, 'probe.cpp')
    obj = os.path.join(WORK, 'probe.o')
    txt = os.path.join(WORK, 'probe.txt')
    with open(src, 'w', encoding='utf-8') as f:
        f.write(src_text)
    ok, log = harness.compile_draft(src, obj)
    if not ok:
        print('=== %s === COMPILE FAILED' % label)
        print(log[:1500])
        return None
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print('=== %s === DISASM FAILED' % label)
        print(dlog[:800])
        return None
    with open(txt, encoding='utf-8', errors='replace') as f:
        text = f.read()
    # find the function body matching fn_pattern
    m = re.search(r'\.fn\s+"?([^",]+)"?.*?\n(.*?)\.endfn', text, re.S)
    print('=== %s ===' % label)
    if m:
        body_lines = [l for l in m.group(2).splitlines() if l.strip()]
        for l in body_lines:
            print('  ' + l.strip())
    else:
        print('  (no function found)')
    return text

if __name__ == '__main__':
    pass
