import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

# name -> target file, and whether it's expected to match
FUNCS = [
    ('__ct__17dBgActorManager_cFv', TARGET),
    ('__dt__17dBgActorManager_cFv', TARGET),
    ('initialize__17dBgActorManager_cFv', TARGET),
    ('create__17dBgActorManager_cFv', TARGET),
    ('CreateHeap__17dBgActorManager_cFv', TARGET),
    ('execute__17dBgActorManager_cFv', TARGET),
    ('ProcMain__17dBgActorManager_cFv', TARGET),
    ('addObj__17dBgActorManager_cFUsUsUsUc', TARGET),
    ('createObjList__17dBgActorManager_cFb', TARGET),
    ('init__Q217dBgActorManager_c7BgObj_cFv', TARGET),
    ('clear__Q217dBgActorManager_c7BgObj_cFv', TARGET),
    ('set__Q217dBgActorManager_c7BgObj_cFUsUsUsUc', TARGET),
    ('createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c', TARGET),
    ('deleteActor__Q217dBgActorManager_c7BgObj_cFv', TARGET),
    ('getOffset__Q217dBgActorManager_c7BgObj_cFv', TARGET),
    ('getSize__Q217dBgActorManager_c7BgObj_cFv', TARGET),
]

for name, target in FUNCS:
    matched, report = harness.diff_fn(target, DIS, name)
    status = 'MATCH' if matched else 'DIFFER'
    lines = report.splitlines()
    # print size line + first few diffs
    print('=== %s [%s] ===' % (name, status))
    for l in lines[:14]:
        print('  ' + l)
    if len(lines) > 14:
        print('  ... (%d more lines)' % (len(lines) - 14))
    print()
