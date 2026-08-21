import os
import re
import subprocess
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
SRC = os.path.join(BASE, 'd_bg_actor_mng.cpp')

# Probe candidate flags that might turn on global-object registration.
CANDIDATES = [
    '-pragma "register_global_object on"',
    '-register_global_object',
]

for flag in CANDIDATES:
    obj = os.path.join(BASE, 'probe.o')
    extra = ['-i', os.path.join(BASE, 'shadow')]
    args = [harness.MWCC] + harness.flags_for('wiimj2d') + [flag, SRC, '-o', obj] + extra + [f'-i{i}' for i in harness.INCLUDES]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    print('=== flag: %s' % flag)
    print('compile rc:', p.returncode)
    if p.returncode == 0:
        dis = os.path.join(BASE, 'probe_disasm.txt')
        harness.disasm(obj, dis)
        names = []
        with open(dis, encoding='utf-8', errors='replace') as fh:
            for line in fh:
                m = re.match(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$', line)
                if m:
                    names.append(m.group(1).strip().strip('"'))
        print('functions:', names)
        # count register calls
        txt = open(dis, encoding='utf-8', errors='replace').read()
        print('register_global_object calls:', txt.count('__register_global_object'))
    else:
        print(p.stdout[-2000:])
        print(p.stderr[-2000:])
