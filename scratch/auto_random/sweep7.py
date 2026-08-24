"""Sweep 7: a/b vec3s declared outside the loop; decl-order permutations."""
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
}

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

@DECLS@
    do {
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
@AB@
        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();

        RandRec &rr = reinterpret_cast<RandRec *>(&buf[0x20])[i];
        rr.x = b.x;
        rr.y = b.y;
        rr.z = b.z;
        raw.x = rp.x;
        raw.y = rp.y;
        rr.raw = raw;
        rr.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
'''

VARIANTS = {}
cases = [
    ('R1_ab_out', '    mVec3_c a;\n    mVec3_c b;\n    mVec2_c raw;\n    int i = 0;',
     '        a = pos;\n        b = a;'),
    ('R2_b_out_a_in', '    mVec3_c b;\n    mVec2_c raw;\n    int i = 0;',
     '        mVec3_c a(pos);\n        b = a;'),
    ('R3_all_in_ctrl', '    mVec2_c raw;\n    int i = 0;',
     '        mVec3_c a(pos);\n        mVec3_c b(a);'),
    ('R4_ab_raw_i', '    mVec3_c a;\n    mVec3_c b;\n    mVec2_c raw;\n    int i = 0;',
     '        a.operator=(pos);\n        b.operator=(a);'),
]
for name, decls, ab in cases:
    src = HEAD.replace('@DECLS@', decls).replace('@AB@', ab)
    VARIANTS[name] = src


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
        print('%-6d %-20s %s' % (n, name, msg))
