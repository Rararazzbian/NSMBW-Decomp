import os, sys, re
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

WORK = os.path.join(ROOT, 'wip', 'nand_thread', 'scratch', 'closer_d')


def try_variant(label, src_text, extra_inc=()):
    src = os.path.join(WORK, 'probe.cpp')
    obj = os.path.join(WORK, 'probe.o')
    txt = os.path.join(WORK, 'probe.txt')
    with open(src, 'w', encoding='utf-8') as f:
        f.write(src_text)
    ok, log = harness.compile_draft(src, obj, extra_inc=extra_inc)
    if not ok:
        print('=== %s === COMPILE FAILED' % label)
        print(log[:2000])
        return None
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print('=== %s === DISASM FAILED' % label)
        print(dlog[:800])
        return None
    with open(txt, encoding='utf-8', errors='replace') as f:
        text = f.read()
    print('=== %s ===' % label)
    # print every function body found
    for m in re.finditer(r'\.fn\s+"?([^",]+)"?.*?\n(.*?)\.endfn', text, re.S):
        name = m.group(1)
        body_lines = [l for l in m.group(2).splitlines() if l.strip() and l.strip().startswith('/*')]
        insns = []
        for l in body_lines:
            mm = re.match(r'/\*.*?\*/\s*(\S.*)$', l.strip())
            if mm:
                insns.append(mm.group(1).strip())
        print('  --- %s (%d insns) ---' % (name, len(insns)))
        for i in insns:
            print('    ' + i)
    return text


if __name__ == '__main__':
    pass
