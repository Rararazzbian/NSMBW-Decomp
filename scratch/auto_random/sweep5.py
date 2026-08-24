"""Sweep 5: vec2 copy chain for record raw-pos fields."""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/scratch/auto_random')
import build
import harness

BASE = build.BASE
FN = build.FN

COMMON = '''#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {
struct RandRec { float x, y, z; {RAWTYPE} raw; float dist; };
}

u32 dRandom_c::calcMachineRandom() {{
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {{
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();

        RandRec &rr = reinterpret_cast<RandRec *>(&buf[0x20])[i];
        rr.x = b.x;
        rr.y = b.y;
        rr.z = b.z;
{RAWWRITE}
        rr.dist = mPad::g_core[i]->getDpdDistance();
    }} while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}}
'''

VARIANTS = {
    'Z1_mvec2_tmp': ('mVec2_c', '''        mVec2_c cur = *reinterpret_cast<const mVec2_c *>(&rp);
        rr.raw = cur;
'''),
    'Z2_eggvec2_tmp': ('EGG::Vector2f', '''        EGG::Vector2f cur = *reinterpret_cast<const EGG::Vector2f *>(&rp);
        rr.raw = cur;
'''),
    'Z3_mvec2_f32ctor': ('mVec2_c', '''        mVec2_c cur(reinterpret_cast<const f32 *>(&rp));
        rr.raw = cur;
'''),
    'Z4_scalar_pair': ('mVec2_c', '''        mVec2_c cur;
        cur.x = rp.x;
        cur.y = rp.y;
        rr.raw = cur;
'''),
    'Z5_direct_assign': ('mVec2_c', '''        rr.raw = *reinterpret_cast<const mVec2_c *>(&rp);
'''),
}


def score(txt):
    t = harness.extract(build.TARGET, FN) or []
    d = harness.extract(txt, FN) or []
    tc = [l.split('*/', 1)[-1].strip() for l in t]
    dc = [l.split('*/', 1)[-1].strip() for l in d]
    diff = sum(1 for i in range(max(len(tc), len(dc)))
               if tc[i:i + 1] != dc[i:i + 1])
    return '%dw/%dw difflines=%d' % (len(d), len(t), diff)


if __name__ == '__main__':
    res = []
    for name, (rt, rw) in sorted(VARIANTS.items()):
        src = COMMON.replace('{RAWTYPE}', rt).replace('{RAWWRITE}', rw)
        src = src.replace('{{', '{').replace('}}', '}')
        p = os.path.join(BASE, 's5_%s.cpp' % name)
        open(p, 'w').write(src)
        ok, log = build.build(p, label='s5_' + name)
        if not ok:
            res.append((99999, name, 'COMPILE FAIL ' + log[-160:].replace('\n', ' ')))
            continue
        sc = score(os.path.join(BASE, 's5_%s.txt' % name))
        res.append((int(sc.split('difflines=')[1]), name, sc))
    for n, name, msg in sorted(res):
        print('%-6d %-22s %s' % (n, name, msg))
