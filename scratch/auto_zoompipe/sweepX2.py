"""Round 2: execute() - FPR decl order x idx-update shape."""
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

HEAD = open(os.path.join(BASE, 'v_A2.cpp')).read().split('int daZoomPipeBase_c::execute()')[0]

BODY_DIRECT = '''            mIdx += 1;
            mIdx &= 1;
            mCurrent = mSpeed[mIdx];
            x = cur;
'''

BODY_NAMED = '''            u8 idx = mIdx;
            idx += 1;
            idx &= 1;
            mIdx = idx;
            mCurrent = mSpeed[idx];
            x = cur;
'''

BODY_NAMED_EXPR = '''            u8 idx = mIdx + 1;
            idx &= 1;
            mIdx = idx;
            mCurrent = mSpeed[idx];
            x = cur;
'''

TWO_BODY = HEAD + '''int daZoomPipeBase_c::execute() {
    if (((dActor_c *) this)->ActorScrOutCheck(0)) {
        return 1;
    }

    @@LOCALS@@

    float x;

    if (tgt <= cur) {
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
L_SCT = 'float step = mStep;\n    float cur = mCurrent;\n    float tgt = mTargetLength;'
L_STC = 'float step = mStep;\n    float tgt = mTargetLength;\n    float cur = mCurrent;'
L_TCS = 'float tgt = mTargetLength;\n    float cur = mCurrent;\n    float step = mStep;'
L_CST = 'float cur = mCurrent;\n    float step = mStep;\n    float tgt = mTargetLength;'
L_CTS = 'float cur = mCurrent;\n    float tgt = mTargetLength;\n    float step = mStep;'

VARIANTS = {}
for tag, loc in [('TSC', L_TSC), ('SCT', L_SCT), ('STC', L_STC),
                 ('TCS', L_TCS), ('CST', L_CST), ('CTS', L_CTS)]:
    VARIANTS['N_' + tag] = TWO_BODY.replace('@@LOCALS@@', loc) \
        .replace('@@G1@@', '!(x >= cur)').replace('@@G2@@', '!(x <= cur)') \
        .replace('@@B@@', BODY_NAMED)
    VARIANTS['P_' + tag] = TWO_BODY.replace('@@LOCALS@@', loc) \
        .replace('@@G1@@', 'x < cur').replace('@@G2@@', 'x > cur') \
        .replace('@@B@@', BODY_NAMED)

VARIANTS['NE_TSC'] = TWO_BODY.replace('@@LOCALS@@', L_TSC) \
    .replace('@@G1@@', '!(x >= cur)').replace('@@G2@@', '!(x <= cur)') \
    .replace('@@B@@', BODY_NAMED_EXPR)


def score(name, src):
    path = os.path.join(BASE, 'w_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(src)
    txt = B.build(path, label='w_%s' % name)
    if not txt:
        print('%-9s COMPILE FAIL' % name)
        return None, txt
    matched, msg = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    head = msg.splitlines()[0]
    ndiff = sum(1 for ln in msg.splitlines() if ' want: ' in ln)
    print('%-9s %s diff-lines=%d%s' % (name, head, ndiff, ' MATCH' if matched else ''))
    return matched, txt


if __name__ == '__main__':
    best = None
    for name in sorted(VARIANTS):
        matched, _ = score(name, VARIANTS[name])
        if matched:
            best = name
            break
    print('BEST:', best)
