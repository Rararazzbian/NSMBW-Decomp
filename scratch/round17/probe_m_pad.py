import os
import re
import subprocess
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
SRC = os.path.join(ROOT, 'wip', 'm_pad', 'scratch', 'batch3', 'm_pad.cpp')
OBJ = os.path.join(BASE, 'm_pad_probe.o')
DIS = os.path.join(BASE, 'm_pad_probe_disasm.txt')

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=(os.path.join(ROOT, 'wip', 'm_pad', 'scratch', 'batch3', 'mock_include'),))
print('COMPILE:', 'OK' if ok else 'FAILED')
if not ok:
    print(log)
    sys.exit(1)
ok2, log2 = harness.disasm(OBJ, DIS)
print('DISASM:', 'OK' if ok2 else 'FAILED')
txt = open(DIS, encoding='utf-8', errors='replace').read()
print('bl __register_global_object:', txt.count('bl __register_global_object'))
print('__construct_array:', txt.count('__construct_array'))
print('__arraydtor:', txt.count('arraydtor'))
