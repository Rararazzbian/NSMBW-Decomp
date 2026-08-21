import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
name = 'probe_delta'
SRC = os.path.join(BASE, name + '.cpp')
OBJ = os.path.join(BASE, name + '.o')
DIS = os.path.join(BASE, name + '_disasm.txt')
ok, log = harness.compile_draft(SRC, OBJ)
print('COMPILE:', 'OK' if ok else 'FAILED')
if not ok:
    print(log)
    sys.exit(1)
harness.disasm(OBJ, DIS)
txt = open(DIS, encoding='utf-8', errors='replace').read()
print('register:', txt.count('bl __register_global_object'),
      'construct_array:', txt.count('__construct_array'),
      'arraydtor:', txt.count('arraydtor'))
for line in txt.splitlines():
    if 'register_global' in line or ('lis' in line and ('l_nested' in line or 'l_top' in line)):
        print(line)
