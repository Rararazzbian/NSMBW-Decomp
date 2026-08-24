"""Sweep 10: statement-order permutations around the rp call."""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/scratch/auto_random')
import build
import harness

BASE = build.BASE
FN = build.FN

HEAD = '''#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {
struct RandRec { float x, y, z; mVec2_c raw; float dist; };
typedef RandRec RandRecArr[4];
}

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);
    mVec3_c a;
    mVec3_c b;
    mVec2_c raw;
    int i = 0;

    do {
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
@aLINE@
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();
@BLINE@
        recs[i].x = b.x;
        recs[i].y = b.y;
        recs[i].z = b.z;
        raw.x = rp.x;
        raw.y = rp.y;
        recs[i].raw = raw;
        recs[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
'''

VARIANTS = {}
cases = [
    ('U1_bafter',  '        a = pos;',            '        b = a;'),
    ('U2_bbef',    '        a = pos;\n        b = a;', ''),
]
for name, aline, bline in cases:
    src = HEAD.replace('@aLINE@', aline).replace('@BLINE@', bline)
    VARIANTS[name] = src

# U3: record writes split around dist call (dist stored last already) — control
# U4: raw assigned via vec2 assign from rp then copied
VARIANTS['U4_rawassign'] = HEAD.replace(
    '@aLINE@', '        a = pos;').replace('@BLINE@', '        b = a;'
).replace('''        raw.x = rp.x;
        raw.y = rp.y;
        recs[i].raw = raw;''',
'''        raw = *reinterpret_cast<const mVec2_c *>(&rp);
        recs[i].raw = raw;''')

# U5: pos/b via ctor chain inside loop (baseline check with new return type)
VARIANTS['U5_ctrs'] = HEAD.replace(
    '@aLINE@', '        mVec3_c a(pos);').replace('@BLINE@', '        mVec3_c b(a);'
).replace('''    RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);
    mVec3_c a;
    mVec3_c b;
    mVec2_c raw;''',
'''    RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);
    mVec2_c raw;''')


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
    for name, src in sorted(VARIANTS.items()):
        p = os.path.join(BASE, '%s.cpp' % name)
        open(p, 'w').write(src)
        ok, log = build.build(p, label=name)
        if not ok:
            res.append((99999, name, 'COMPILE FAIL'))
            continue
        sc = score(os.path.join(BASE, '%s.txt' % name))
        res.append((int(sc.split('difflines=')[1]), name, sc))
    for n, name, msg in sorted(res):
        print('%-6d %-16s %s' % (n, name, msg))
