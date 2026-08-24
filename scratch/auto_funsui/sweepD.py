"""Sweep D: case-3 source shapes for posMove. Case12 held at B6-optimal."""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness
harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_funsui/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_funsui/dtk'
sys.path.insert(0, '/opt/NSMBW-Decomp/scratch/auto_funsui')
import score

BASE = '/opt/NSMBW-Decomp/scratch/auto_funsui'

PRELUDE = '''#include <game/bases/d_a_player_base.hpp>
#include <game/framework/f_manager.hpp>
#include <game/bases/d_funsui_act.hpp>

static inline u8 &field_38c_ref(daPlBase_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x38c);
}

void dFunsuiAct_c::posMove() {
    static const float cs_rev_speed[2] = { 3.0f, -3.0f };
    daPlBase_c *p = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);
    if (p == NULL) {
        return;
    }

    switch (field_38c_ref(p)) {
'''

CASE12 = '''    case 1:
    case 2: {
        daPlBase_c *pl = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);

        if (mTimer > 0) {
            mTimer--;
            mTargetY += cs_rev_speed[mRevIndex];
            mSpeed = 0.0f;
        } else {
            mTargetY = pl->mPos.x;
        }
        mCurrentY += mSpeed;
        pl->updateFunsuiPos(mTargetY, mCurrentY);
        break;
    }
'''

EPILOG = '''    }
}
'''

# ---- case 3 bodies ------------------------------------------------------

def writes_inter():
    return '''        float px = p->mPos.x;
        float nc = mCurrentY + mSpeed;
        mTargetY = px;
        mCurrentY = nc;
        p->mPos.x = px;
        p->mPos.y = nc;
'''

def writes_direct():
    return '''        mTargetY = p->mPos.x;
        mCurrentY += mSpeed;
        p->mPos.x = mTargetY;
        p->mPos.y = mCurrentY;
'''

CASE3 = {}

# named locals, various orders/forms
CASE3['n_inter_c1c2'] = writes_inter() + '''        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();
        mVec3_c v(c1.x, c2.y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
CASE3['n_direct_c1c2'] = writes_direct() + '''        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();
        mVec3_c v(c1.x, c2.y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
# constructor form: two direct calls
CASE3['t_inter_ctor'] = writes_inter() + '''        mVec3_c v(p->getCenterPos().x, p->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
CASE3['t_direct_ctor'] = writes_direct() + '''        mVec3_c v(p->getCenterPos().x, p->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
# floats bound after the calls
CASE3['f_inter_floats'] = writes_inter() + '''        float vx = p->getCenterPos().x;
        float vy = p->getCenterPos().y;
        mVec3_c v(vx, vy, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
CASE3['f_direct_floats'] = writes_direct() + '''        float vx = p->getCenterPos().x;
        float vy = p->getCenterPos().y;
        mVec3_c v(vx, vy, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
# declared-then-assigned vectors
CASE3['a_inter_assign'] = writes_inter() + '''        mVec3_c c1;
        mVec3_c c2;
        c1 = p->getCenterPos();
        c2 = p->getCenterPos();
        mVec3_c v;
        v.x = c1.x;
        v.y = c2.y;
        v.z = 5500.0f;
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
# reversed call order feeding swapped fields
CASE3['n_inter_rev'] = writes_inter() + '''        mVec3_c c2 = p->getCenterPos();
        mVec3_c c1 = p->getCenterPos();
        mVec3_c v(c1.x, c2.y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
# base-class-named receiver
CASE3['b_direct_base'] = writes_direct() + '''        dBaseActor_c *a = p;
        mVec3_c v(a->getCenterPos().x, a->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''
# reference receiver
CASE3['r_inter_ref'] = writes_inter() + '''        daPlBase_c &q = *p;
        mVec3_c v(q.getCenterPos().x, q.getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''


def make(name, case3):
    src = PRELUDE + CASE12 + '    case 3: {\n' + case3 + '        break;\n    }\n' + EPILOG
    path = os.path.join(BASE, name + '.cpp')
    open(path, 'w').write(src)
    return path


def main():
    results = []
    inc = (os.path.join(BASE, 'shadow'),)
    for key, body in sorted(CASE3.items()):
        name = 'swD_' + key
        src = make(name, body)
        ok, log = harness.compile_draft(src, os.path.join(BASE, name + '.o'), extra_inc=inc, module='wiimj2d')
        if not ok:
            print('%-22s COMPILE FAIL' % name)
            continue
        ok, log = harness.disasm(os.path.join(BASE, name + '.o'), os.path.join(BASE, name + '.txt'))
        if not ok:
            print('%-22s DISASM FAIL' % name)
            continue
        msg = score.score(os.path.join(BASE, name + '.txt'), verbose=False)
        results.append((name, msg))
        print('%-22s %s' % (name, msg))
    best = max(results, key=lambda r: int(r[1].split('match ')[1]))
    print('BEST:', best[0])


if __name__ == '__main__':
    main()
