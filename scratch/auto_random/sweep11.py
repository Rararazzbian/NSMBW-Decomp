"""Sweep 11 (final): cores ref-binding and two-index loop forms."""
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
'''

BODY = '''        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(@CC@) + 0x24);
        a = pos;
        b = a;
        EGG::Vector2f rp = @CC@->getDpdRawPos();

        recs[@RI@].x = b.x;
        recs[@RI@].y = b.y;
        recs[@RI@].z = b.z;
        raw.x = rp.x;
        raw.y = rp.y;
        recs[@RI@].raw = raw;
        recs[@RI@].dist = @CC@->getDpdDistance();
'''

TAIL = '''
    return OSCalcCRC32(buf, 0x80);
}
'''

VARIANTS = {}

# V2: cores bound through array reference
src = COMMON + '''    EGG::CoreController *const (&cores)[4] = mPad::g_core;
    int i = 0;

    do {
''' + BODY.replace('@CC@', 'cores[i]').replace('@RI@', 'i') + '''    } while (++i <= 3);
''' + TAIL
VARIANTS['V2_coresref'] = src

# V3: two-index loop
src = COMMON + '''    int ci = 0;
    int ri = 0;

    do {
''' + BODY.replace('@CC@', 'mPad::g_core[ci]').replace('@RI@', 'ri') + '''    } while (++ci <= 3 && ++ri < 8);
''' + TAIL
VARIANTS['V3_twoidx'] = src

# V4: control (current best shape)
src = COMMON + '''    int i = 0;

    do {
''' + BODY.replace('@CC@', 'mPad::g_core[i]').replace('@RI@', 'i') + '''    } while (++i <= 3);
''' + TAIL
VARIANTS['V4_ctrl'] = src


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
