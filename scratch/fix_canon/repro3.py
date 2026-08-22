import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness

target = harness.extract('wip/agent_line_mng/work/target.txt', 'executeState_Left30Left__10dLineMng_cFv')

for draftpath in ['wip/fix_bigtwo/_tally/d.txt', 'wip/fix_bigtwo/_tally_cmp/d.txt']:
    draft = harness.extract(draftpath, 'executeState_Left30Left__10dLineMng_cFv')
    print(draftpath, 'lines:', len(draft) if draft else None, 'EQUAL:', target == draft if (target and draft) else None)
