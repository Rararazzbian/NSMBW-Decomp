import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'round28', 'd_bg_ctr')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

src = os.path.join(BASE, 'd_bg_ctr.cpp')
obj = os.path.join(BASE, 'd_bg_ctr.o')
dis = os.path.join(BASE, 'draft_disasm.txt')
extra = (os.path.join(BASE, 'shadow'),)
ok, log = harness.compile_draft(src, obj, extra_inc=extra)
print('COMPILE:', ok)
if log:
    print(log)
if not ok:
    raise SystemExit(1)
ok, log = harness.disasm(obj, dis)
print('DISASM:', ok, log)
if not ok:
    raise SystemExit(1)

target = os.path.join(BASE, 'target.txt')
for name in ['calc__9dBg_ctr_cFv', 'fn_8007FFA0', 'revisePos__9dBg_ctr_cFv',
             'addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c', 'fn_80080900',
             'fn_80080E40', 'fn_80080670__9dBg_ctr_cFP7mVec3_cf']:
    got = harness.extract(dis, name)
    if got is None and name.startswith('fn_'):
        for line in open(dis, encoding='utf-8', errors='replace'):
            if line.startswith('.fn ' + name + '__'):
                actual = line.split()[1].rstrip(',')
                got = harness.extract(dis, actual)
                break
    want = harness.extract(target, name)
    if want is None and name.startswith('fn_'):
        want = harness.extract(target, name)
    if got is None or want is None:
        print(name, 'MISSING', len(want or []), len(got or []))
    else:
        print(name, len(want), len(got), 'MATCH' if want == got else 'DIFFER')
