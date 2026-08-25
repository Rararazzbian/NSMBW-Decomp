"""Round 4: execute() - corrected arrival guards x locals order."""
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

BODY_DIRECT = '''            mIdx += 1;
            mIdx &= 1;
            mCurrent = mSpeed[mIdx];
            x = cur;
'''

TPL = HEAD + '''int daZoomPipeBase_c::execute() {
    if (((dActor_c *) this)->ActorScrOutCheck(0)) {
        return 1;
    }

    @@LOCALS@@

    float x;

    if (@@OUTER@@) {
        x = tgt + step;
        if (@@G1@@) {
@@B@@        }
    } else {
        x = tgt - step;
        if (@@G2@@) {
@@B@@        }
    }

    calcDownLength(x);
    return daObjPipeBase_c::execute();
}
'''

L_TSC = 'float tgt = mTargetLength;\n    float step = mStep;\n    float cur = mCurrent;'
L_CTS = 'float cur = mCurrent;\n    float tgt = mTargetLength;\n    float step = mStep;'
L_STC = 'float step = mStep;\n    float tgt = mTargetLength;\n    float cur = mCurrent;'
L_TCS = 'float tgt = mTargetLength;\n    float cur = mCurrent;\n    float step = mStep;'
L_CST = 'float cur = mCurrent;\n    float step = mStep;\n    float tgt = mTargetLength;'
L_SCT = 'float step = mStep;\n    float cur = mCurrent;\n    float tgt = mTargetLength;'

VARIANTS = {}
for tag, loc in [('TSC', L_TSC), ('CTS', L_CTS), ('STC', L_STC),
                 ('TCS', L_TCS), ('CST', L_CST), ('SCT', L_SCT)]:
    VARIANTS['G_' + tag] = (TPL.replace('@@LOCALS@@', loc)
                            .replace('@@OUTER@@', 'tgt <= cur')
                            .replace('@@G1@@', 'x >= cur').replace('@@G2@@', 'x <= cur')
                            .replace('@@B@@', BODY_NAMED))
    VARIANTS['H_' + tag] = (TPL.replace('@@LOCALS@@', loc)
                            .replace('@@OUTER@@', 'tgt <= cur')
                            .replace('@@G1@@', 'x >= cur').replace('@@G2@@', 'x <= cur')
                            .replace('@@B@@', BODY_DIRECT))


def score(name):
    path = os.path.join(BASE, 'g_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(VARIANTS[name])
    txt = B.build(path, label='g_%s' % name)
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
