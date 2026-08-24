"""Sweep F: probe-local variants to place the dead float pair at r1+0x8/0xc."""
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

TAIL = '''        dBaseActor_c *a = p;
        mVec3_c v(a->getCenterPos().x, a->getCenterPos().y, 5500.0f);
        mEffect.createEffect("Wm_en_quicksand", 0, &v, NULL, NULL);
'''

CASE3 = {}

# baseline e1 shape for reference
CASE3['g1_baseline'] = '''        float px = p->mPos.x;
        float nc = mCurrentY + mSpeed;
        mTargetY = px;
        mCurrentY = nc;
        p->mPos.x = px;
        p->mPos.y = nc;
''' + TAIL

# probe filled AFTER writes (e5) but y assigned first
CASE3['g2_swapassign'] = '''        float px = p->mPos.x;
        float nc = mCurrentY + mSpeed;
        mTargetY = px;
        mCurrentY = nc;
        p->mPos.x = px;
        p->mPos.y = nc;
        mVec2_c probe;
        probe.y = nc;
        probe.x = px;
''' + TAIL

# probe declared first, filled from expressions, writes read back through it
CASE3['g3_probe_first'] = '''        mVec2_c probe;
        probe.x = p->mPos.x;
        probe.y = mCurrentY + mSpeed;
        float px = probe.x;
        float nc = probe.y;
        mTargetY = px;
        mCurrentY = nc;
        p->mPos.x = px;
        p->mPos.y = nc;
''' + TAIL

# everything through the probe, no separate floats
CASE3['g4_probe_only'] = '''        mVec2_c probe;
        probe.x = p->mPos.x;
        probe.y = mCurrentY + mSpeed;
        mTargetY = probe.x;
        mCurrentY = probe.y;
        p->mPos.x = probe.x;
        p->mPos.y = probe.y;
''' + TAIL

# probe constructed first from expressions, then writes through it
CASE3['g5_probe_ctor1st'] = '''        mVec2_c probe(p->mPos.x, mCurrentY + mSpeed);
        mTargetY = probe.x;
        mCurrentY = probe.y;
        p->mPos.x = probe.x;
        p->mPos.y = probe.y;
''' + TAIL

# POD variant of g2
CASE3['g6_pod'] = '''        float px = p->mPos.x;
        float nc = mCurrentY + mSpeed;
        mTargetY = px;
        mCurrentY = nc;
        p->mPos.x = px;
        p->mPos.y = nc;
        mVec2_POD_c probe;
        probe.set(px, nc);
''' + TAIL

# probe ctor right after computing values, writes still via floats
CASE3['g7_ctor_mid'] = '''        float px = p->mPos.x;
        float nc = mCurrentY + mSpeed;
        mVec2_c probe(px, nc);
        mTargetY = px;
        mCurrentY = nc;
        p->mPos.x = px;
        p->mPos.y = nc;
''' + TAIL

# swapped fields probe (x=nc, y=px) -- allocation-direction probe
CASE3['g8_xfields_swapped'] = '''        float px = p->mPos.x;
        float nc = mCurrentY + mSpeed;
        mTargetY = px;
        mCurrentY = nc;
        p->mPos.x = px;
        p->mPos.y = nc;
        mVec2_c probe;
        probe.x = nc;
        probe.y = px;
''' + TAIL


def make(name, case3):
    src = PRELUDE + CASE12 + '    case 3: {\n' + case3 + '        break;\n    }\n' + EPILOG
    path = os.path.join(BASE, name + '.cpp')
    open(path, 'w').write(src)
    return path


def main():
    results = []
    inc = (os.path.join(BASE, 'shadow'),)
    for key, body in sorted(CASE3.items()):
        name = 'swF_' + key[3:]
        src = make(name, body)
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
