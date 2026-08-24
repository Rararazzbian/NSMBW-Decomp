"""Sweep 18: force integer-IV addressing (lwzx base+idx / addi r27+add r27).

Target signature we are hunting:
  lis/addi g_core base ONCE; lwzx r4, r29, r30        (fixed base + byte IV)
  addi r27, r1, 0x50 ; add r27, r27, r31              (per-iter base rebuild)
  six DISPLACEMENT stores off r27 (never stfsux/stfsx)
  _savegpr_27/_restgpr_27 prologue (five callee-saved GPRs)

All variants share identical statement order; only the record-address
expression and small declaration details vary.
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw18')
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

typedef RandRec RandRecArr[4];

} // namespace

u32 dRandom_c::calcMachineRandom() {
'''

TAIL = '''    return OSCalcCRC32(buf, 0x80);
}
'''


def std_loop(rc_bind):
    """Shared loop skeleton. rc_bind: source text binding 'rc', or None."""
    s = '''    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

'''
    if rc_bind:
        s += '    int i = 0;\n    do {\n'
        s += '        %s\n' % rc_bind
    else:
        s += '    int i = 0;\n    do {\n'
    s += '''        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

'''
    if rc_bind:
        s += '''        rc.x = b.x;
        rc.y = b.y;
        rc.z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rc.raw = raw;

        rc.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

'''
    else:
        s += '''        f6[i * 6 + 0] = b.x;
        f6[i * 6 + 1] = b.y;
        f6[i * 6 + 2] = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        f6[i * 6 + 3] = raw.x;
        f6[i * 6 + 4] = raw.y;

        f6[i * 6 + 5] = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

'''
    return s


V = {}

V['v1_refidx'] = HEAD + std_loop(
    'RandRec &rc = ((RandRec *)&buf[0x20])[i];') + TAIL

V['v2_chararith'] = HEAD + std_loop(
    'RandRec &rc = *(RandRec *)(buf + 0x20 + i * 0x18);') + TAIL

V['v3_mulsizeof'] = HEAD + std_loop(
    'RandRec &rc = *(RandRec *)(buf + 0x20 + i * sizeof(RandRec));') + TAIL

V['v4_floatsix'] = HEAD + '''    float *f6 = reinterpret_cast<float *>(&buf[0x20]);
''' + std_loop(None) + TAIL

V['v5_helper'] = HEAD.replace('} // namespace\n\nu32 dRandom_c::calcMachineRandom() {\n',
'''struct Collector {
    static void collectOne(float *dstBase, int i) {
        EGG::CoreController *cc = mPad::g_core[i];
        const mVec3_c &pos =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(pos);
        mVec3_c b(a);
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        dstBase[i * 6 + 0] = b.x;
        dstBase[i * 6 + 1] = b.y;
        dstBase[i * 6 + 2] = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        dstBase[i * 6 + 3] = raw.x;
        dstBase[i * 6 + 4] = raw.y;

        dstBase[i * 6 + 5] = mPad::g_core[i]->getDpdDistance();
    }
};

} // namespace

u32 dRandom_c::calcMachineRandom() {
''') + '''    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
        Collector::collectOne(reinterpret_cast<float *>(&buf[0x20]), i);
    } while (++i <= 3);

''' + TAIL

V['v6_arrtype'] = HEAD + std_loop(
    'RandRec &rc = (*reinterpret_cast<RandRecArr (*)[4]>(&buf[0x20]))[i];') + TAIL

V['v7_hoisted'] = HEAD + '''    RandRecArr &recs = *reinterpret_cast<RandRecArr *>(&buf[0x20]);
''' + std_loop('RandRec &rc = recs[i];') + TAIL

V['v8_offvar'] = HEAD + '''    char *base = buf + 0x20;
    int off = 0;
''' + std_loop('RandRec &rc = *(RandRec *)(base + off);').replace(
    '    } while (++i <= 3);\n',
    '        off += 0x18;\n    } while (++i <= 3);\n') + TAIL


def gen(name, src):
    p = os.path.join(OUT, name + '.cpp')
    with open(p, 'w') as f:
        f.write(src)
    obj = os.path.join(OUT, name + '.o')
    txt = os.path.join(OUT, name + '.txt')
    ok, log = harness.compile_draft(p, obj,
                                    extra_inc=(os.path.join(BASE, 'shadow'),),
                                    module='wiimj2d')
    if not ok:
        return name, None, 'BUILDFAIL ' + str(log).strip().splitlines()[-1][:80]
    ok, _ = harness.disasm(obj, txt)
    if not ok:
        return name, None, 'DISASMFAIL'
    want = harness.extract(TARGET, FN)
    got = harness.extract(txt, FN) or []
    text = '\n'.join(got)
    full = open(txt).read()
    sig = []
    if 'lwzx' in text:
        sig.append('lwzx')
    if '_savegpr_27' in full:
        sig.append('SAVE27')
    if 'stfsux' in text:
        sig.append('stfsux!')
    if re.search(r'addi r\d+, r1, 0x50.{0,40}add r\d+, r\d+, r\d+',
                 text, re.S):
        sig.append('REBUILD')
    if not got:
        return name, None, 'NOFN ' + ' '.join(sig)
    matched, msg = harness.diff_fn(TARGET, txt, FN)[:2]
    if matched:
        return name, 0, 'MATCH ' + ' '.join(sig)
    nd = sum(1 for k in range(max(len(want), len(got)))
             if (want[k] if k < len(want) else None) !=
                (got[k] if k < len(got) else None))
    return name, nd, '%dw %s %s' % (len(got), str(msg).splitlines()[0][:28],
                                    ' '.join(sig))


if __name__ == '__main__':
    rows = [gen(n, s) for n, s in sorted(V.items())]
    rows.sort(key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999))
    for n, d, m in rows:
        print('%-14s %-6s %s' % (n, d, m))
