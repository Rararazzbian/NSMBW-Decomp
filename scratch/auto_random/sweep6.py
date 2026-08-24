"""Sweep 6b: out-of-loop raw vec2 + record addressing combos (token replacement)."""
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
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();

@RRLINE@
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

RR_INLOOP = '        RandRec &rr = reinterpret_cast<RandRec *>(&buf[0x20])[i];'
RR_PTR = ('        float *recs = reinterpret_cast<float *>(&buf[0x20]);\n'
          '        RandRec &rr = reinterpret_cast<RandRec *>(recs)[i];')

VARIANTS = {}
for dname, decls in [
    ('raw_i', '    mVec2_c raw;\n    int i = 0;'),
    ('i_raw', '    int i = 0;\n    mVec2_c raw;'),
]:
    for rname, rrline in [('rrin', RR_INLOOP), ('rrptr', RR_PTR)]:
        src = HEAD.replace('@DECLS@', decls).replace('@RRLINE@', rrline)
        VARIANTS['Q_%s_%s' % (dname, rname)] = src


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
        print('%-6d %-24s %s' % (n, name, msg))
