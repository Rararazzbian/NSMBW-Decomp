"""Mega-sweep 9: permutations over decl order, rr placement, subscript styles."""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/scratch/auto_random')
import build
import harness

BASE = build.BASE
FN = build.FN

HEADER = '''#include <game/bases/d_random.hpp>
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
@RR@
        const mVec3_c &pos = @POSEXPR@;
@aB@
        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();

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

POS_CHAR = ('*reinterpret_cast<const mVec3_c *>(\n'
            '            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24)')
POS_F32 = 'mVec3_c(reinterpret_cast<const f32 *>(mPad::g_core[i]) + 9)'

RR_ARRREF = '        RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);'
RR_NONE = ''

VARIANTS = {}

decl_orders = [
    ('o1', ['RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);',
            'mVec3_c a;', 'mVec3_c b;', 'mVec2_c raw;', 'int i = 0;']),
    ('o2', ['mVec3_c a;', 'mVec3_c b;', 'mVec2_c raw;', 'int i = 0;',
            'RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);']),
    ('o3', ['mVec2_c raw;', 'mVec3_c a;', 'mVec3_c b;', 'int i = 0;']),
    ('o4', ['int i = 0;', 'mVec2_c raw;', 'mVec3_c a;', 'mVec3_c b;']),
    ('o5', ['mVec3_c a;', 'mVec2_c raw;', 'mVec3_c b;', 'int i = 0;']),
]

for oname, items in decl_orders:
    has_recarr = any('RandRecArr &recs' in s for s in items)
    decls = '    ' + '\n    '.join(items)
    for pname, posexpr in [('ch', POS_CHAR), ('f32', None)]:
        if pname == 'f32':
            continue
        ab = '        a = pos;\n        b = a;'
        rr_inline = ''
        src = (HEADER
               .replace('@DECLS@', decls)
               .replace('@RR@', rr_inline)
               .replace('@POSEXPR@', posexpr)
               .replace('@aB@', ab))
        # o1 puts recs decl in DECLS already; ensure body uses recs only via decls
        key = 'S_%s' % oname
        VARIANTS[key] = src

# variant where recs binding lives INSIDE loop (before pos)
src_inloop = HEADER.replace(
    '@DECLS@',
    '    mVec3_c a;\n    mVec3_c b;\n    mVec2_c raw;\n    int i = 0;'
).replace('@RR@',
    '        RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);'
).replace('@POSEXPR@', POS_CHAR).replace('@aB@', '        a = pos;\n        b = a;')
VARIANTS['S_inloop_rr'] = src_inloop


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
