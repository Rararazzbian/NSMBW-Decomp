import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness

target = harness.extract('wip/agent_line_mng/work/target.txt', 'executeState_Left30Left__10dLineMng_cFv')
draft = harness.extract('wip/fix_bigtwo/_tally/d.txt', 'executeState_Left30Left__10dLineMng_cFv')

print("target lines:", len(target) if target else None)
print("draft lines:", len(draft) if draft else None)

if target and draft:
    print("EQUAL:", target == draft)
    for i in range(max(len(target), len(draft))):
        a = target[i] if i < len(target) else '<none>'
        b = draft[i] if i < len(draft) else '<none>'
        if a != b:
            print("DIFF @%d:\n  target: %r\n  draft : %r" % (i, a, b))
