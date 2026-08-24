"""Sweep 3: direct-buf indexing and slot-rederive shapes."""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/scratch/auto_random')
import build
import harness

BASE = build.BASE
FN = build.FN

PRELUDE = '''#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
'''

POSTLUDE = '''    } while (++i <= 3);

    return OSCalcCRC32(buf, 0x80);
}
'''

POS = ('        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(\n'
       '            reinterpret_cast<const char *>({CC}) + 0x24);\n'
       '        mVec3_c a(pos);\n'
       '        mVec3_c b(a);\n')

VARIANTS = {}

# W1: float* recs pointer (current baseline)
VARIANTS['W1_floathdr'] = PRELUDE + (
    POS.format(CC='mPad::g_core[i]') +
    '        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();\n'
    '        float *r = reinterpret_cast<float *>(&buf[0x20]) + i * 6;\n'
    '        r[0] = b.x;\n        r[1] = b.y;\n        r[2] = b.z;\n'
    '        r[3] = rp.x;\n        r[4] = rp.y;\n'
    '        r[5] = mPad::g_core[i]->getDpdDistance();\n') + POSTLUDE

# W2: fully direct byte-offset writes on buf
VARIANTS['W2_directbuf'] = PRELUDE + (
    POS.format(CC='mPad::g_core[i]') +
    '        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x00) = b.x;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x04) = b.y;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x08) = b.z;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x0c) = rp.x;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x10) = rp.y;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x14) = mPad::g_core[i]->getDpdDistance();\n') + POSTLUDE

# W3: slot re-derived via g_core + i, direct buf writes
SLOT = ('        EGG::CoreController *const *slot = mPad::g_core + i;\n')
VARIANTS['W3_slot_direct'] = PRELUDE + (
    SLOT +
    POS.format(CC='*slot') +
    '        EGG::Vector3f rp = (*slot)->getDpdRawPos();\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x00) = b.x;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x04) = b.y;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x08) = b.z;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x0c) = rp.x;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x10) = rp.y;\n'
    '        *reinterpret_cast<float *>(buf + 0x20 + i * 0x18 + 0x14) = (*slot)->getDpdDistance();\n') + POSTLUDE

# W4: slot rederived, Rec struct via cast per iteration
VARIANTS['W4_slot_recaddr'] = PRELUDE + (
    SLOT +
    POS.format(CC='*slot') +
    '        EGG::Vector3f rp = (*slot)->getDpdRawPos();\n'
    '        Rec_ &rr = reinterpret_cast<Rec_ *>(&buf[0x20])[i];\n'
    '        rr.x = b.x;\n        rr.y = b.y;\n        rr.z = b.z;\n'
    '        rr.rx = rp.x;\n        rr.ry = rp.y;\n'
    '        rr.dist = (*slot)->getDpdDistance();\n') + POSTLUDE

RECDEF = '''
struct Rec_ { float x, y, z, rx, ry, dist; };
'''
for k in ('W4_slot_recaddr',):
    VARIANTS[k] = VARIANTS[k].replace('u32 dRandom_c::calcMachineRandom', RECDEF + '\nu32 dRandom_c::calcMachineRandom', 1)

# W5: baseline + le tail (control)
VARIANTS['W5_ctrl_le'] = PRELUDE + (
    POS.format(CC='mPad::g_core[i]') +
    '        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();\n'
    '        float *recs = reinterpret_cast<float *>(&buf[0x20]);\n'
    '        recs[i * 6 + 0] = b.x;\n        recs[i * 6 + 1] = b.y;\n        recs[i * 6 + 2] = b.z;\n'
    '        recs[i * 6 + 3] = rp.x;\n        recs[i * 6 + 4] = rp.y;\n'
    '        recs[i * 6 + 5] = mPad::g_core[i]->getDpdDistance();\n') + POSTLUDE


def score(txt):
    t = harness.extract(build.TARGET, FN) or []
    d = harness.extract(txt, FN) or []
    tc = [l.split('*/', 1)[-1].strip() for l in t]
    dc = [l.split('*/', 1)[-1].strip() for l in d]
    diff = sum(1 for i in range(max(len(tc), len(dc)))
               if tc[i:i + 1] != dc[i:i + 1])
    return '%dw/%dw difflines=%d' % (len(d), len(t), diff), len(d), len(t)


if __name__ == '__main__':
    res = []
    for name, src in sorted(VARIANTS.items()):
        p = os.path.join(BASE, 's3_%s.cpp' % name)
        open(p, 'w').write(src)
        ok, log = build.build(p, label='s3_' + name)
        if not ok:
            res.append((99999, name, 'COMPILE FAIL'))
            continue
        sc, nd, nt = score(os.path.join(BASE, 's3_%s.txt' % name))
        res.append((int(sc.split('difflines=')[1]), name, sc))
    for n, name, msg in sorted(res):
        print('%-6d %-22s %s' % (n, name, msg))
