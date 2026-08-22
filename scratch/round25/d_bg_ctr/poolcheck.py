import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round25', 'd_bg_ctr')
OBJ = os.path.join(BASE, 'd_bg_ctr.o')
TARGET = os.path.join(BASE, 'target.txt')

ok, report = harness.poolcheck(OBJ, TARGET)
print(report)