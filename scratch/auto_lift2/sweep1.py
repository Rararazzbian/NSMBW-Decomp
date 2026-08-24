import sys, subprocess
sys.path.insert(0, '/opt/NSMBW-Decomp/scratch/auto_lift2')
import build

HEAD = '''#include <game/bases/d_lift_allhit_draw2.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_bg.hpp>

extern const float l_poolZero_8042CA70;
extern const float l_poolOne_8042CA90;

void fn_800BFB60(void *, float *, u8, u8, u8, s16);

'''

FN_TAIL = '''
        fn_800BFB60(mUnk22ac, &v.x, a, b, c, ang);
    }
}
'''

MID_BLOCKS = {
 'D_merged': '''\
        f32 z = mMaxY;
        f32 mid = mMidY;
        f32 y = mMinY;
        mVec3_c v(y, mid, z);
        f32 bg = dBg_c::m_bg_p->mLoopOffset;
        if (y < l_poolZero_8042CA70) {
            v.x = y + bg;
        } else {
            v.x = y - bg;
        }
''',
 'A_compound_pre': '''\
        f32 z = mMaxY;
        f32 mid = mMidY;
        f32 y = mMinY;
        mVec3_c v(y, mid, z);
        f32 bg = dBg_c::m_bg_p->mLoopOffset;
        v.x = y;
        if (y < l_poolZero_8042CA70) {
            v.x += bg;
        } else {
            v.x -= bg;
        }
''',
 'B_compound_only': '''\
        f32 z = mMaxY;
        f32 mid = mMidY;
        f32 y = mMinY;
        mVec3_c v(y, mid, z);
        f32 bg = dBg_c::m_bg_p->mLoopOffset;
        if (y < l_poolZero_8042CA70) {
            v.x += bg;
        } else {
            v.x -= bg;
        }
''',
 'C_two_ifs': '''\
        f32 z = mMaxY;
        f32 mid = mMidY;
        f32 y = mMinY;
        mVec3_c v(y, mid, z);
        f32 bg = dBg_c::m_bg_p->mLoopOffset;
        if (y < l_poolZero_8042CA70) {
            v.x = y + bg;
        }
        if (y >= l_poolZero_8042CA70) {
            v.x = y - bg;
        }
''',
}

for name, blk in MID_BLOCKS.items():
    src = HEAD + 'void dLiftAllhitDraw2_c::draw() {\n' + \
        '''    u8 a = mColA;
    u8 b = mColB;
    u8 c = mColC;
    s16 ang = mAng;

    fn_800BFB60(mUnk1c, &mMinY, a, b, c, ang);

    if (dScStage_c::m_loopType == 1) {
''' + blk + FN_TAIL
    p = '/opt/NSMBW-Decomp/scratch/auto_lift2/sw_%s.cpp' % name
    open(p, 'w').write(src)
    ok, r = build.build('sw_%s.cpp' % name, 'sw_%s' % name)
    if not ok:
        print('%-18s COMPILE FAIL' % name)
        continue
    m, msg = build.harness.diff_fn(build.TARGET,
        '/opt/NSMBW-Decomp/scratch/auto_lift2/sw_%s.txt' % name,
        'draw__18dLiftAllhitDraw2_cFv')
    print('%-18s %s' % (name, 'MATCH ***' if m else msg.splitlines()[0] +
          ' | ' + ' ; '.join(msg.splitlines()[1:4])))
