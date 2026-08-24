"""Sweep 17: attack the two remaining strength-reduction decisions.

Target facts (from target.txt):
  - g_core[i] read twice via lwzx r4/r3, r29(base)+r30(i*4); NO moving pointer
  - record address rebuilt EVERY iteration: addi r27, r1, 0x50; add r27, r27, r31
    (r31 = i*0x18 accumulated), all six stores displacement-form off r27
  - counter r28 compared cmpwi 3; r31 += 0x18, r30 += 4 at tail

Variants:
  v1_slotref : CoreController *const &cc = g_core[i] bound inside loop (each
               use re-reads the slot); recs ref bound inside loop.
  v2_twoiv   : manual byte-offset variable advanced alongside i (compiler may
               refuse to merge two independent IVs into one moving pointer).
  v3_u8ctr   : u8 loop counter.
  v4_split   : H_collect3 helper shape but source order split exactly as
               target: xyz stores, then rp->raw copy, then dist (no contiguous
               6-store run for stfsux to chew on).
"""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw17')
os.makedirs(OUT, exist_ok=True)
TARGET = '/opt/NSMBW-Decomp/tools/auto_decomp/work/dol_bases_d_random/target.txt'
FN = 'calcMachineRandom__9dRandom_cFv'

HEAD = '''#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace {

struct RandRec {
    float x, y, z;
    mVec2_c raw;
    float dist;
};

'''

PRELUDE = '''    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

'''

TAIL = '''    return OSCalcCRC32(buf, 0x80);
}
'''


def v1():
    return PRELUDE + '''    int i = 0;
    do {
        EGG::CoreController *const &cc = mPad::g_core[i];
        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector2f rp = cc->getDpdRawPos();

        RandRec &rc = reinterpret_cast<RandRec *>(&buf[0x20])[i];
        rc.x = b.x;
        rc.y = b.y;
        rc.z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rc.raw = raw;

        rc.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

''' + TAIL


def v2():
    return PRELUDE + '''    int i = 0;
    int off = 0;
    do {
        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        RandRec &rc = *reinterpret_cast<RandRec *>(
            reinterpret_cast<char *>(buf) + 0x20 + off);
        rc.x = b.x;
        rc.y = b.y;
        rc.z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rc.raw = raw;

        rc.dist = mPad::g_core[i]->getDpdDistance();
        ++i;
        off += sizeof(RandRec);
    } while (i <= 3);

''' + TAIL


def v3():
    return PRELUDE + '''    u8 i = 0;
    do {
        EGG::CoreController *const &cc = mPad::g_core[i];
        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector2f rp = cc->getDpdRawPos();

        RandRec &rc = reinterpret_cast<RandRec *>(&buf[0x20])[i];
        rc.x = b.x;
        rc.y = b.y;
        rc.z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rc.raw = raw;

        rc.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

''' + TAIL


def v4():
    helper = '''struct Collector {
    static void collectOne(float *dstBase, int i) {
        EGG::CoreController *cc = mPad::g_core[i];
        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector2f rp = cc->getDpdRawPos();

        float *dst = dstBase + i * 6;
        dst[0] = b.x;
        dst[1] = b.y;
        dst[2] = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        dst[3] = raw.x;
        dst[4] = raw.y;

        dst[5] = mPad::g_core[i]->getDpdDistance();
    }
};

'''
    return HEAD + helper + '} // namespace\n\nu32 dRandom_c::calcMachineRandom() {\n' \
        + PRELUDE + '''    int i = 0;
    do {
        Collector::collectOne(reinterpret_cast<float *>(&buf[0x20]), i);
    } while (++i <= 3);

''' + TAIL


def gen(name, body):
    src = HEAD + '} // namespace\n\nu32 dRandom_c::calcMachineRandom() {\n' + body
    p = os.path.join(OUT, name + '.cpp')
    with open(p, 'w') as f:
        f.write(src)
    obj = os.path.join(OUT, name + '.o')
    txt = os.path.join(OUT, name + '.txt')
    ok, log = harness.compile_draft(p, obj,
                                    extra_inc=(os.path.join(BASE, 'shadow'),),
                                    module='wiimj2d')
    if not ok:
        return name, None, 'BUILDFAIL ' + str(log).strip().splitlines()[-1][:90]
    ok, _ = harness.disasm(obj, txt)
    if not ok:
        return name, None, 'DISASMFAIL'
    matched, msg = harness.diff_fn(TARGET, txt, FN)[:2]
    want = harness.extract(TARGET, FN)
    got = harness.extract(txt, FN) or []
    if matched:
        return name, 0, 'MATCH'
    nd = sum(1 for k in range(max(len(want), len(got)))
             if (want[k] if k < len(want) else '<none>') !=
                (got[k] if k < len(got) else '<none>'))
    return name, nd, '%dw %s' % (len(got), str(msg).splitlines()[1].strip()[:44])


if __name__ == '__main__':
    rows = [
        gen('v1_slotref', v1()),
        gen('v2_twoiv', v2()),
        gen('v3_u8ctr', v3()),
        gen('v4_split', v4()),
    ]
    rows.sort(key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999))
    for n, d, m in rows:
        print('%-12s %-6s %s' % (n, d, m))
