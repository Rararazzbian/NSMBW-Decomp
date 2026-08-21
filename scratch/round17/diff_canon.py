import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

# canonicalise-aware diff: extract both, canonicalise, compare
for name in ('initialize__17dBgActorManager_cFv', 'execute__17dBgActorManager_cFv'):
    print('=== %s ===' % name)
    tgt = harness.extract(TARGET, name)
    drf = harness.extract(DIS, name)
    if tgt is None or drf is None:
        print('  MISSING:', 'target' if tgt is None else 'draft')
        continue
    ct = harness.canonicalise(tgt)
    cd = harness.canonicalise(drf)
    print('  canonical target len %d, draft len %d' % (len(ct), len(cd)))
    if ct == cd:
        print('  CANONICAL MATCH')
    else:
        for i, (a, b) in enumerate(zip(ct, cd)):
            if a != b:
                print('  line %d:\n    want: %s\n    got:  %s' % (i, a, b))
                if i > 40:
                    break
