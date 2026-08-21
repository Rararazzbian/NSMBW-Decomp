import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))

import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
SRC = os.path.join(BASE, 'd_bg_actor_mng.cpp')
OBJ = os.path.join(BASE, 'd_bg_actor_mng.o')
DISASM = os.path.join(BASE, 'draft_disasm.txt')

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=(os.path.join(BASE, 'shadow'),))
print('COMPILE:', 'OK' if ok else 'FAILED')
if log:
    print(log)

if ok:
    ok2, log2 = harness.disasm(OBJ, DISASM)
    print('DISASM:', 'OK' if ok2 else 'FAILED', log2)

    names = []
    with open(DISASM, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = re.match(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$', line)
            if m:
                names.append(m.group(1).strip().strip('"'))
    print('\nEMITTED FUNCTIONS (%d):' % len(names))
    for n in names:
        print('  %s' % n)
