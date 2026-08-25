"""Round 3: execute() - operand-order and polarity spellings."""
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


def mk(name, loc, outer, g1, g2):
    src = (TPL.replace('@@LOCALS@@', loc).replace('@@OUTER@@', outer)
           .replace('@@G1@@', g1).replace('@@G2@@', g2).replace('@@B@@', BODY_NAMED))
    VARIANTS[name] = src


VARIANTS = {}
mk('R1', L_TSC, 'cur >= tgt', '!(cur <= x)', '!(cur >= x)')
mk('R2', L_CTS, 'cur >= tgt', '!(cur <= x)', '!(cur >= x)')
mk('R3', L_TSC, 'tgt <= cur', '!(cur <= x)', '!(cur >= x)')
mk('R4', L_TSC, 'cur >= tgt', '!(x >= cur)', '!(x <= cur)')
mk('R5', L_TCS, 'cur >= tgt', '!(cur <= x)', '!(cur >= x)')
mk('R6', L_STC, 'cur >= tgt', '!(cur <= x)', '!(cur >= x)')


def score(name):
    src = VARIANTS[name]
    path = os.path.join(BASE, 'r_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(src)
    txt = B.build(path, label='r_%s' % name)
    if not txt:
        print('%-4s COMPILE FAIL' % name)
        return False
    matched, msg = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    head = msg.splitlines()[0]
    ndiff = sum(1 for ln in msg.splitlines() if ' want: ' in ln)
    print('%-4s %s diff-lines=%d%s' % (name, head, ndiff, ' MATCH' if matched else ''))
    return matched


if __name__ == '__main__':
    for n in sorted(VARIANTS):
        if score(n):
            print('WINNER:', n)
            break
