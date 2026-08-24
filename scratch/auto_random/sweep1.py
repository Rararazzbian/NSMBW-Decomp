"""Variant sweep for calcMachineRandom — compile N shapes, score against target."""
import os
import subprocess
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/scratch/auto_random')
import build
import harness

BASE = build.BASE

HEAD = '''#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {
struct Rec { float x, y, z, rx, ry, dist; };
}

u32 dRandom_c::calcMachineRandom() {
'''

BODY_TMPL = '''    Rec recs[4];
{BUF}
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

{LOOPHEAD} {{
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();
{DEAD}
        recs[i].x = b.x;
        recs[i].y = b.y;
        recs[i].z = b.z;
        recs[i].rx = rp.x;
        recs[i].ry = rp.y;
        recs[i].dist = mPad::g_core[i]->getDpdDistance();
    }}{LOOPTAIL}

    return OSCalcCRC32(buf, 0x80);
}}
'''

VARIANTS = {}
for loopname, head, tail in [
    ('dowhile', '    int i = 0;\n    do', ' while (++i < 4);'),
    ('forlt4', '    for (int i = 0; i < 4; i++)', ''),
    ('forle3', '    for (int i = 0; i <= 3; i++)', ''),
]:
    for bufdecl in ['buf-first', 'buf-none']:
        for dead in ['dead-no', 'dead-vec2']:
            name = '%s_%s_%s' % (loopname, bufdecl, dead)
            BUF = '' if bufdecl == 'buf-first' else ''
            # buf-first means declared before recs; we model by swapping lines below
            deadline = '\n        EGG::Vector2f rpXY(rp.x, rp.y);' if dead == 'dead-vec2' else ''
            body = BODY_TMPL.format(BUF='', LOOPHEAD=head, LOOPTAIL=tail, DEAD=deadline)
            if bufdecl == 'buf-first':
                body = body.replace('    Rec recs[4];\n', '    char buf[0x80];\n    Rec recs[4];\n')
            VARIANTS[name] = HEAD + body


def score(txt):
    t = harness.extract(build.TARGET, build.FN) or []
    d = harness.extract(txt, build.FN) or []
    tc = [l.split('*/', 1)[-1].strip() for l in t]
    dc = [l.split('*/', 1)[-1].strip() for l in d]
    n = sum(1 for i in range(max(len(tc), len(dc)))
            if tc[i:i+1] != dc[i:i+1])
    return len(tc), len(dc), sum(1 for a, b in zip(tc, dc) if a != b)


if __name__ == '__main__':
    results = []
    for name, src in sorted(VARIANTS.items()):
        p = os.path.join(BASE, 'v_%s.cpp' % name)
        open(p, 'w').write(src)
        ok, log = build.build(p, label='v_' + name)
        if not ok:
            results.append((10**9, name, 'COMPILE FAIL'))
            continue
        txt = os.path.join(BASE, 'v_%s.txt' % name)
        tt, td, diffn = score(txt)
        results.append((diffn + abs(tt - td), name, '%dw/%dw %d difflines' % (td, tt, diffn)))
    for sc, name, msg in sorted(results):
        print('%-6d %-28s %s' % (sc, name, msg))
