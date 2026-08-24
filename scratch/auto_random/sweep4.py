"""Sweep 4: union overlay + scoped dead vec2 probes."""
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

'''

POS = ('        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(\n'
       '            reinterpret_cast<const char *>({CC}) + 0x24);\n'
       '        mVec3_c a(pos);\n'
       '        mVec3_c b(a);\n'
       '        EGG::Vector3f rp = {CC2}->getDpdRawPos();\n')

VARIANTS = {}

# X1: W5 baseline + scoped dead vec2
body = (
    POS.format(CC='mPad::g_core[i]', CC2='mPad::g_core[i]') +
    '        {\n'
    '            EGG::Vector2f rawXY(rp.x, rp.y);\n'
    '            (void)rawXY;\n'
    '        }\n'
    '        float *recs = reinterpret_cast<float *>(&buf[0x20]);\n'
    '        recs[i * 6 + 0] = b.x;\n        recs[i * 6 + 1] = b.y;\n        recs[i * 6 + 2] = b.z;\n'
    '        recs[i * 6 + 3] = rp.x;\n        recs[i * 6 + 4] = rp.y;\n'
    '        recs[i * 6 + 5] = mPad::g_core[i]->getDpdDistance();\n')
VARIANTS['X1_scopedvec2'] = COMMON + '''
u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
''' + body + '''    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
'''

# X2: unscoped dead vec2 placed BETWEEN rx and ry stores
body2 = (
    POS.format(CC='mPad::g_core[i]', CC2='mPad::g_core[i]') +
    '        float *recs = reinterpret_cast<float *>(&buf[0x20]);\n'
    '        recs[i * 6 + 0] = b.x;\n        recs[i * 6 + 1] = b.y;\n        recs[i * 6 + 2] = b.z;\n'
    '        recs[i * 6 + 3] = rp.x;\n'
    '        EGG::Vector2f rawXY(rp.x, rp.y);\n'
    '        recs[i * 6 + 4] = rp.y;\n'
    '        recs[i * 6 + 5] = mPad::g_core[i]->getDpdDistance();\n')
VARIANTS['X2_vec2mid'] = COMMON + '''
u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
''' + body2 + '''    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
'''

# Y1: union overlay
Y1 = COMMON + '''
namespace {
struct RandRec { float x, y, z, rx, ry, dist; };
struct RandBlock {
    s64 time;
    char nick[0x18];
    RandRec recs[4];
};
}

u32 dRandom_c::calcMachineRandom() {
    union {
        char bytes[0x80];
        RandBlock d;
    } u;

    memset(u.bytes, 0, 0x80);
    u.d.time = OSGetTime();
    SCGetOwnerNickName(u.d.nick);

    int i = 0;
    do {
''' + POS.format(CC='mPad::g_core[i]', CC2='mPad::g_core[i]') + '''
        u.d.recs[i].x = b.x;
        u.d.recs[i].y = b.y;
        u.d.recs[i].z = b.z;
        u.d.recs[i].rx = rp.x;
        u.d.recs[i].ry = rp.y;
        u.d.recs[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(u.bytes, 0x80);
}
'''
VARIANTS['Y1_union'] = Y1

# Y2: struct overlay via pointer cast of buf, records typed
Y2 = COMMON + '''
namespace {
struct RandRec { float x, y, z, rx, ry, dist; };
}

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
''' + POS.format(CC='mPad::g_core[i]', CC2='mPad::g_core[i]') + '''
        RandRec &rr = reinterpret_cast<RandRec *>(&buf[0x20])[i];
        rr.x = b.x;
        rr.y = b.y;
        rr.z = b.z;
        rr.rx = rp.x;
        rr.ry = rp.y;
        rr.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
'''
VARIANTS['Y2_recaddr'] = Y2

# Y3: Y2 + vec2 between
Y3 = Y2.replace(
    '        rr.rx = rp.x;\n        rr.ry = rp.y;\n',
    '        rr.rx = rp.x;\n'
    '        EGG::Vector2f rawXY(rp.x, rp.y);\n'
    '        rr.ry = rp.y;\n')
VARIANTS['Y3_recaddr_vec2'] = Y3


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
        p = os.path.join(BASE, 's4_%s.cpp' % name)
        open(p, 'w').write(src)
        ok, log = build.build(p, label='s4_' + name)
        if not ok:
            res.append((99999, name, 'COMPILE FAIL: ' + log[-200:].replace('\n', ' ')))
            continue
        sc = score(os.path.join(BASE, 's4_%s.txt' % name))
        res.append((int(sc.split('difflines=')[1]), name, sc))
    for n, name, msg in sorted(res):
        print('%-6d %-22s %s' % (n, name, msg))
