import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET_SINIT = os.path.join(BASE, 'target_sinit_d_bg_actor_mn.txt')
TARGET_TAIL = os.path.join(BASE, 'target_8007F6D4.txt')

# The target's dtor name is __dt__Q217dBgActorManager_c11BgObjName_tFv
# The draft's arraydtors are __arraydtor$4380..4386 (fresh numbers each build)

def diff(name, target):
    matched, report = harness.diff_fn(target, DIS, name)
    print('=== %s [%s] ===' % (name, 'MATCH' if matched else 'DIFFER'))
    # report may be long; print first/last 40 lines
    lines = report.splitlines()
    print('\n'.join(lines[:30]))
    if len(lines) > 60:
        print('... (%d more lines)' % (len(lines) - 60))
        print('\n'.join(lines[-30:]))
    else:
        print('\n'.join(lines[30:]))

diff('__sinit_\\d_bg_actor_mng_cpp', TARGET_SINIT)
