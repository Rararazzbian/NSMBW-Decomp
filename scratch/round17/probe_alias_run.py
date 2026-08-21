import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
SRC = os.path.join(BASE, 'probe_alias.cpp')
OBJ = os.path.join(BASE, 'probe_alias.o')
DIS = os.path.join(BASE, 'probe_alias_disasm.txt')

ok, log = harness.compile_draft(SRC, OBJ)
print('COMPILE:', 'OK' if ok else 'FAILED')
if not ok:
    print(log)
    sys.exit(1)
harness.disasm(OBJ, DIS)
txt = open(DIS, encoding='utf-8', errors='replace').read()
print('bl __register_global_object:', txt.count('bl __register_global_object'))
print('__construct_array:', txt.count('__construct_array'))
print('__arraydtor:', txt.count('arraydtor'))
