"""Sweep case12/case3 source orderings for posMove, score against target."""
import os
import re
import subprocess
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness
harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_funsui/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_funsui/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_funsui'
TARGET = os.path.join(BASE, '..', '..', 'tools/auto_decomp/work/dol_bases_d_funsui_act/target.txt')
TARGET = os.path.normpath(TARGET)

HEADER_PRELUDE = '''#include <game/bases/d_a_player_base.hpp>
#include <game/framework/f_manager.hpp>
#include <game/bases/d_funsui_act.hpp>

extern const float l_zero_8042C848;
extern const float l_5500_8042C84C;
extern const char l_str_803158C8[16];
extern const float l_revSpeed_8042C840[2];

static inline u8 &field_38c_ref(daPlBase_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x38c);
}

void dFunsuiAct_c::posMove() {
    daPlBase_c *p = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);
    if (p == NULL) {
        return;
    }

    switch (field_38c_ref(p)) {
'''

CASE12 = {
 'c1': '''        if (mTimer > 0) {
            mTimer--;
            mSpeed = l_zero_8042C848;
            mTargetY += l_revSpeed_8042C840[mRevIndex];
        } else {
            mTargetY = pl->mPos.x;
        }
        mCurrentY += mSpeed;
        pl->updateFunsuiPos(mTargetY, mCurrentY);
        break;''',
 'c2': '''        if (mTimer > 0) {
            mTimer--;
            float rv = l_revSpeed_8042C840[mRevIndex];
            mTargetY += rv;
            mSpeed = l_zero_8042C848;
        } else {
            mTargetY = pl->mPos.x;
        }
        mCurrentY += mSpeed;
        pl->updateFunsuiPos(mTargetY, mCurrentY);
        break;''',
 'c3': '''        if (mTimer > 0) {
            float rv = l_revSpeed_8042C840[mRevIndex];
            mTimer--;
            mSpeed = l_zero_8042C848;
            mTargetY += rv;
        } else {
            mTargetY = pl->mPos.x;
        }
        mCurrentY += mSpeed;
        pl->updateFunsuiPos(mTargetY, mCurrentY);
        break;''',
}

CASE3 = {
 'k1': '''    case 3: {
        float nc = mCurrentY + mSpeed;
        mTargetY = p->mPos.x;
        mCurrentY = nc;
        p->mPos.x = mTargetY;
        p->mPos.y = nc;

        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();

        mVec3_c v(c1.x, c2.y, l_5500_8042C84C);
        mEffect.createEffect(l_str_803158C8, 0, &v, NULL, NULL);
        break;
    }''',
 'k2': '''    case 3: {
        float nc = mCurrentY + mSpeed;
        mTargetY = p->mPos.x;
        mCurrentY = nc;
        p->mPos.x = mTargetY;
        p->mPos.y = nc;

        mVec3_c v;
        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();

        v.x = c1.x;
        v.y = c2.y;
        v.z = l_5500_8042C84C;
        mEffect.createEffect(l_str_803158C8, 0, &v, NULL, NULL);
        break;
    }''',
 'k3': '''    case 3: {
        float nc = mSpeed + mCurrentY;
        mTargetY = p->mPos.x;
        mCurrentY = nc;
        p->mPos.x = mTargetY;
        p->mPos.y = nc;

        mVec3_c v;
        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();

        v.x = c1.x;
        v.y = c2.y;
        v.z = l_5500_8042C84C;
        mEffect.createEffect(l_str_803158C8, 0, &v, NULL, NULL);
        break;
    }''',
 'k4': '''    case 3: {
        float nc = mCurrentY + mSpeed;
        mTargetY = p->mPos.x;
        mCurrentY = nc;
        p->mPos.x = mTargetY;
        p->mPos.y = nc;

        mVec3_c v;
        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();
        mVec3_c d;
        d.x = mTargetY;
        d.y = mCurrentY;

        v.x = c1.x;
        v.y = c2.y;
        v.z = l_5500_8042C84C;
        mEffect.createEffect(l_str_803158C8, 0, &v, NULL, NULL);
        break;
    }''',
 'k5': '''    case 3: {
        float nc = mCurrentY + mSpeed;
        mTargetY = p->mPos.x;
        mCurrentY = nc;
        p->mPos.x = mTargetY;
        p->mPos.y = nc;

        mVec3_c v;
        mVec3_c c1 = p->getCenterPos();
        mVec3_c c2 = p->getCenterPos();
        mVec2_c d;
        d.x = mTargetY;
        d.y = mCurrentY;

        v.x = c1.x;
        v.y = c2.y;
        v.z = l_5500_8042C84C;
        mEffect.createEffect(l_str_803158C8, 0, &v, NULL, NULL);
        break;
    }''',
}

def build_src(c12, c3):
    mid = '''    case 1:
    case 2: {
        daPlBase_c *pl = (daPlBase_c *)fManager_c::searchBaseByID(mBaseId);

'''
    return HEADER_PRELUDE + mid + CASE12[c12] + '\n' + CASE3[c3] + '\n    }\n    }\n}\n'

def main():
    results = []
    for ck in ('c1', 'c2', 'c3'):
        for kk in ('k1', 'k2', 'k3', 'k4', 'k5'):
            src = build_src(ck, kk)
            path = os.path.join(BASE, 'sw_%s_%s.cpp' % (ck, kk))
            with open(path, 'w') as fh:
                fh.write(src)
            obj = os.path.join(BASE, 'sw_%s_%s.o' % (ck, kk))
            txt = os.path.join(BASE, 'sw_%s_%s.txt' % (ck, kk))
            ok, log = harness.compile_draft(path, obj,
                                            extra_inc=(os.path.join(BASE, 'shadow'),),
                                            module='wiimj2d')
            if not ok:
                results.append('%s %s COMPILE-FAIL' % (ck, kk))
                continue
            ok, log = harness.disasm(obj, txt)
            if not ok:
                results.append('%s %s DISASM-FAIL' % (ck, kk))
                continue
            matched, msg = harness.diff_fn(TARGET, txt, 'posMove__12dFunsuiAct_cFv')
            m = re.search(r'target (\d+), draft (\d+)', msg or '')
            ndiff = len([l for l in (msg or '').splitlines() if '| want:' in l])
            tag = 'MATCH!!!' if matched else ''
            results.append('%s %s  words=%s/%s  difflines=%d %s' %
                           (ck, kk, m.group(1) if m else '?',
                            m.group(2) if m else '?', ndiff, tag))
    print('\n'.join(results))

if __name__ == '__main__':
    main()
