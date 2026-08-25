"""Sweep execute() shapes for daZoomPipeBase_c. Tokens @@X@@ replaced per variant."""
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

HEAD = '''#include <game/bases/d_a_zoom_pipe_base.hpp>

void daZoomPipeBase_c::init(u32 param) {
    u32 w = mParam;

    mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;
    mSpeed[1] = (w & 0xF) * 16.0f + 16.0f;
    mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;
    mCurrent = mSpeed[0];

    fn_80045A10(param, mSpeed[1], (float) ((w >> 16) & 3), 1, 0, 0);
}

int daZoomPipeBase_c::execute() {
    if (((dActor_c *) this)->ActorScrOutCheck(0)) {
        return 1;
    }
'''

BODY_COMPOUND = '''            mIdx += 1;
            mIdx &= 1;
            mCurrent = mSpeed[mIdx];
            x = cur;
'''

BODY_EXPR = '''                mIdx = (mIdx + 1) & 1;
                mCurrent = mSpeed[mIdx];
                x = cur;
'''

TWO_BODY = HEAD + '''
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

ELSE_FORM = HEAD + '''
    @@LOCALS@@
    float x;

    if (tgt <= cur) {
        x = tgt + step;
        if (x >= cur) {
        } else {
@@B@@        }
    } else {
        x = tgt - step;
        if (x <= cur) {
        } else {
@@B@@        }
    }

    calcDownLength(x);
    return daObjPipeBase_c::execute();
}
'''

BOOL_SHARED = HEAD + '''
    @@LOCALS@@
    float x;
    bool arrived;

    if (tgt <= cur) {
        x = tgt + step;
        arrived = @@C1@@;
    } else {
        x = tgt - step;
        arrived = @@C2@@;
    }

    if (!arrived) {
        mIdx += 1;
        mIdx &= 1;
        mCurrent = mSpeed[mIdx];
        x = cur;
    }

    calcDownLength(x);
    return daObjPipeBase_c::execute();
}
'''

L_CTS = 'float cur = mCurrent;\n    float tgt = mTargetLength;\n    float step = mStep;'
L_STC = 'float step = mStep;\n    float cur = mCurrent;\n    float tgt = mTargetLength;'
L_CT = 'float cur = mCurrent;\n    float tgt = mTargetLength;'
L_TSC = 'float tgt = mTargetLength;\n    float step = mStep;\n    float cur = mCurrent;'
L_TCS = 'float tgt = mTargetLength;\n    float cur = mCurrent;\n    float step = mStep;'
L_SCT = 'float step = mStep;\n    float tgt = mTargetLength;\n    float cur = mCurrent;'

VARIANTS = {
    'A1': TWO_BODY.replace('@@LOCALS@@', L_CTS).replace('@@G1@@', 'x < cur').replace('@@G2@@', 'x > cur').replace('@@B@@', BODY_COMPOUND),
    'A2': TWO_BODY.replace('@@LOCALS@@', L_CTS).replace('@@G1@@', '!(x >= cur)').replace('@@G2@@', '!(x <= cur)').replace('@@B@@', BODY_COMPOUND),
    'A3': TWO_BODY.replace('@@LOCALS@@', L_STC).replace('@@G1@@', 'x < cur').replace('@@G2@@', 'x > cur').replace('@@B@@', BODY_COMPOUND),
    'A4': TWO_BODY.replace('@@LOCALS@@', L_TSC).replace('@@G1@@', 'x < cur').replace('@@G2@@', 'x > cur').replace('@@B@@', BODY_COMPOUND),
    'A5': TWO_BODY.replace('@@LOCALS@@', L_TCS).replace('@@G1@@', 'x < cur').replace('@@G2@@', 'x > cur').replace('@@B@@', BODY_COMPOUND),
    'A6': TWO_BODY.replace('@@LOCALS@@', L_SCT).replace('@@G1@@', 'x < cur').replace('@@G2@@', 'x > cur').replace('@@B@@', BODY_COMPOUND),
    'A7': TWO_BODY.replace('@@LOCALS@@', L_CT).replace('@@G1@@', 'x < cur').replace('@@G2@@', 'x > cur').replace('@@B@@', BODY_EXPR),
    'B1': ELSE_FORM.replace('@@LOCALS@@', L_CTS).replace('@@B@@', BODY_COMPOUND),
    'C1': BOOL_SHARED.replace('@@LOCALS@@', L_CT).replace('@@C1@@', 'x >= cur').replace('@@C2@@', 'cur < x'),
}


def score(name, src):
    path = os.path.join(BASE, 'v_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(src)
    txt = B.build(path, label='v_%s' % name)
    if not txt:
        print('%-6s COMPILE FAIL' % name)
        return
    matched, msg = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    head = msg.splitlines()[0]
    ndiff = sum(1 for ln in msg.splitlines() if ' want: ' in ln)
    tag = 'MATCH' if matched else ''
    print('%-6s %s diff-lines=%d %s' % (name, head, ndiff, tag))


if __name__ == '__main__':
    for name in sorted(VARIANTS):
        score(name, VARIANTS[name])
