import os, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

with open(TARGET) as f:
    lines = f.readlines()

start = None
for i, line in enumerate(lines):
    if 'createObjList__17dBgActorManager_cFb' in line and '.fn' in line:
        start = i
    if start is not None and '.endfn' in line and i > start:
        for l in lines[start:i+1]:
            print(l.rstrip())
        break
