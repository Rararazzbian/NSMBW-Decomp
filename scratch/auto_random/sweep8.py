"""Sweep 8: array-reference records + cached cores pointer."""
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

@DECLS@
    do {
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(@CORES@[i]) + 0x24);
        a = pos;
        b = a;
        EGG::Vector3f rp = @CORES@[i]->getDpdRawPos();

        recs[i].x = b.x;
        recs[i].y = b.y;
        recs[i].z = b.z;
        raw.x = rp.x;
        raw.y = rp.y;
        recs[i].raw = raw;
        recs[i].dist = @CORES@[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
'''

VARIANTS = {}
decl_cases = [
    ('T1', '    RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);\n'
           '    EGG::CoreController *a_core;\n'
           '    mVec3_c a;\n    mVec3_c b;\n    mVec2_c raw;\n    int i = 0;',
     'mPad::g_core'),
]
for name, decls, cores in decl_cases:
    src = HEAD.replace('@DECLS@', decls).replace('@CORES@', cores)
    VARIANTS[name] = src

# T2: no cores local, direct g_core
VARIANTS['T2_nocores'] = VARIANTS['T1'].replace(
    '    EGG::CoreController *a_core;\n', '')

# T3: cores pointer-typed local const
VARIANTS['T3_coreslocal'] = HEAD.replace('@DECLS@',
    '    EGG::CoreController *const cores = *reinterpret_cast<EGG::CoreController *(*)[4]>(&mPad::g_core)[0] ;\n'
    '    RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);\n'
    '    mVec3_c a;\n    mVec3_c b;\n    mVec2_c raw;\n    int i = 0;'
).replace('@CORES@', 'cores').replace(
    '*reinterpret_cast<EGG::CoreController *(*)[4]>(&mPad::g_core)[0]',
    'mPad::g_core')


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
            res.append((99999, name, 'COMPILE FAIL ' + log[-200:].replace('\n', ' ')))
            continue
        sc = score(os.path.join(BASE, '%s.txt' % name))
        res.append((int(sc.split('difflines=')[1]), name, sc))
    for n, name, msg in sorted(res):
        print('%-6d %-16s %s' % (n, name, msg))
