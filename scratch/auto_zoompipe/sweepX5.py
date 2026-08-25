"""Round 5: execute() - split declaration from assignment."""
import os
import sys

ROOT = '/opt/NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'auto_zoompipe')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))

import importlib.util
_spec = importlib.util.spec_from_file_location('Bld', os.path.join(BASE, 'build.py'))
B = importlib.util.module_from_spec(_spec)
sys.modules['B'] = B
_spec.loader.exec_module(B)

HEAD = open(os.path.join(BASE, 'w_N_TSC.cpp')).read().split('int daZoomPipeBase_c::execute()')[0]

BODY_NAMED = '''            u8 idx = mIdx;
            idx += 1;
            idx &= 1;
            mIdx = idx;
            mCurrent = mSpeed[idx];
            x = cur;
'''

TPL = HEAD + '''int daZoomPipeBase_c::execute() {
    if (((dActor_c *) this)->ActorScrOutCheck(0)) {
        return 1;
    }

    float tgt;
    float step;
    float cur;

@@ASSIGN@@
    float x;

    if (tgt <= cur) {
        x = tgt + step;
        if (x >= cur) {
@@B@@        }
    } else {
        x = tgt - step;
        if (x <= cur) {
@@B@@        }
    }

    calcDownLength(x);
    return daObjPipeBase_c::execute();
}
'''

A_CST = '    cur = mCurrent;\n    step = mStep;\n    tgt = mTargetLength;'
A_CTS = '    cur = mCurrent;\n    tgt = mTargetLength;\n    step = mStep;'
A_TSC = '    tgt = mTargetLength;\n    step = mStep;\n    cur = mCurrent;'
A_SCT = '    step = mStep;\n    cur = mCurrent;\n    tgt = mTargetLength;'
A_STC = '    step = mStep;\n    tgt = mTargetLength;\n    cur = mCurrent;'
A_TCS = '    tgt = mTargetLength;\n    cur = mCurrent;\n    step = mStep;'

VARIANTS = {}
for tag, a in [('CST', A_CST), ('CTS', A_CTS), ('TSC', A_TSC),
               ('SCT', A_SCT), ('STC', A_STC), ('TCS', A_TCS)]:
    VARIANTS['S_' + tag] = TPL.replace('@@ASSIGN@@', a).replace('@@B@@', BODY_NAMED)


def score(name):
    path = os.path.join(BASE, 's_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(VARIANTS[name])
    txt = B.build(path, label='s_%s' % name)
    if not txt:
        print('%-6s COMPILE FAIL' % name)
        return False
    matched, msg = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    head = msg.splitlines()[0]
    ndiff = sum(1 for ln in msg.splitlines() if ' want: ' in ln)
    print('%-6s %s diff-lines=%d%s' % (name, head, ndiff, ' MATCH' if matched else ''))
    return matched


if __name__ == '__main__':
    winner = None
    for n in sorted(VARIANTS):
        if score(n):
            winner = n
            break
    print('WINNER:', winner)
