"""Sweep E: intermediates x receiver-rename, plus dead-spill probes."""
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

INTER = '''        float px = p->mPos.x;
        float nc = mCurrentY + mSpeed;
        mTargetY = px;
        mCurrentY = nc;
        p->mPos.x = px;
        p->mPos.y = nc;
'''

CASE3 = {}

CASE3['e1_baseptr'] = INTER + '''        dBaseActor_c *a = p;
        mVec3_c v(a->getCenterPos().x, a->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''

CASE3['e2_baseref'] = INTER + '''        dBaseActor_c &a = *p;
        mVec3_c v(a.getCenterPos().x, a.getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''

CASE3['e3_inlinecast'] = INTER + '''        mVec3_c v(((dBaseActor_c *)p)->getCenterPos().x,
                  ((dBaseActor_c *)p)->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''

CASE3['e4_freshname'] = INTER + '''        daPlBase_c *q = p;
        mVec3_c v(q->getCenterPos().x, q->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''

# dead-spill probes: an unused mVec2_c filled with px/nc before the calls
CASE3['e5_probe_assign'] = INTER + '''        mVec2_c probe;
        probe.x = px;
        probe.y = nc;
        dBaseActor_c *a = p;
        mVec3_c v(a->getCenterPos().x, a->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''

CASE3['e6_probe_ctor'] = INTER + '''        mVec2_c probe(px, nc);
        dBaseActor_c *a = p;
        mVec3_c v(a->getCenterPos().x, a->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''

CASE3['e7_topdecl'] = '''        dBaseActor_c *a = p;
''' + INTER + '''        mVec3_c v(a->getCenterPos().x, a->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''

CASE3['e8_fieldassign'] = INTER + '''        dBaseActor_c *a = p;
        mVec3_c v;
        v.x = a->getCenterPos().x;
        v.y = a->getCenterPos().y;
        v.z = 5500.0f;
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
        name = 'swE_' + key.split('_', 1)[1] if False else 'swE_' + key
        src = make(name.replace('swE_e', 'swE_'), body)
        obj = os.path.join(BASE, name + '.o')
        ok, log = harness.compile_draft(src, obj, extra_inc=inc, module='wiimj2d')
        if not ok:
            print('%-22s COMPILE FAIL' % name)
            continue
        ok, log = harness.disasm(obj, os.path.join(BASE, name + '.txt'))
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
