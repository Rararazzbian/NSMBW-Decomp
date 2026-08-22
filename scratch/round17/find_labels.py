import os
BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
with open(os.path.join(BASE, 'draft_disasm.txt')) as f:
    lines = f.readlines()
start = None
for i, line in enumerate(lines):
    if 'ProcMain__17dBgActorManager_cFv' in line and '.fn' in line:
        start = i
    if start is not None and '.endfn' in line and i > start:
        for l in lines[start:i+1]:
            s = l.strip()
            if s.startswith('.L_') or s.startswith('L_'):
                print(repr(s.split()[0]))
        break
