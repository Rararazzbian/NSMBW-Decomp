"""Sweep 19: whole-seed local struct {u64 time; char nick[0x18]; Rec cores[4]}.

Rationale (from frame-layout analysis):
  r1+0x30 = &data (memset 0x80)   r1+0x38 = data.nick
  r1+0x50 = data.cores            8+0x18+4*0x18 = 0x80 exactly.
  Fixed-base + byte-IV addressing for BOTH arrays, counter in r28,
  five callee-saved GPRs -> _savegpr_27, frame 0xd0.

Extraction shape fixed to v4_split's proven form:
  const mVec3_c &r = *(...)(cc+0x24); mVec3_c a(r); mVec3_c b(a);
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw19')
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
    %s
};

struct RandData {
    u64 time;
    char nick[0x18];
    RandRec cores[4];
};

} // namespace

u32 dRandom_c::calcMachineRandom() {
'''

REC_MVEC = '''    mVec3_c pos;
    mVec2_c raw;
    float dist;'''

REC_FLT = '''    float x, y, z;
    mVec2_c raw;
    float dist;'''


def body(rec, extract, timeform, ccspell):
    s = HEAD % rec
    s += '''    RandData data;
    memset(&data, 0, %s);
    %s;
    SCGetOwnerNickName(data.nick);

    int i = 0;
    do {
%s
        EGG::Vector2f rp = %s->getDpdRawPos();

%s

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
%s
%s
    } while (++i <= 3);

    return OSCalcCRC32(&data, 0x80);
}
'''
    return s % ('sizeof(RandData)' , timeform, extract,
                ccspell, '', '', '')


def fill(rec, extract, timeform='data.time = OSGetTime()', ccspell='mPad::g_core[i]',
         posw='data.cores[i].pos = b;', raww='data.cores[i].raw = raw;',
         distw='data.cores[i].dist = %s->getDpdDistance();'):
    s = HEAD % rec
    s += '''    RandData data;
    memset(&data, 0, sizeof(RandData));
    %s;
    SCGetOwnerNickName(data.nick);

    int i = 0;
    do {
%s
        EGG::Vector2f rp = %s->getDpdRawPos();

        %s

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        %s
        %s
    } while (++i <= 3);

    return OSCalcCRC32(&data, 0x80);
}
'''
    dist = distw % ccspell
    return s % (timeform, extract, ccspell, posw, raww, dist)


EX_SA = '''        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(%s) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);'''

EX_SB = '''        const mVec3_c p =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(%s) + 0x24);
        mVec3_c b(p);'''

EX_CC = '''        EGG::CoreController *cc = mPad::g_core[i];
        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);'''


V = {}
V['p1_mvec_cc'] = fill(REC_MVEC, EX_CC % 'cc', ccspell='cc')
V['p2_mvec_inline'] = fill(REC_MVEC, EX_SA % 'mPad::g_core[i]')
V['p3_flt_inline'] = fill(REC_FLT, EX_SA % 'mPad::g_core[i]')
V['p4_flt_cc'] = fill(REC_FLT, EX_CC % 'cc', ccspell='cc')
V['p5_sb'] = fill(REC_MVEC, EX_SB % 'mPad::g_core[i]')
V['p6_timecast'] = fill(REC_MVEC, EX_SA % 'mPad::g_core[i]',
                        timeform='*(s64 *)&data = OSGetTime()')
V['p7_memset_lit'] = fill(REC_MVEC, EX_SA % 'mPad::g_core[i]').replace(
    'memset(&data, 0, sizeof(RandData));', 'memset(&data, 0, 0x80);')
V['p8_fieldflt'] = fill(REC_FLT, EX_SA % 'mPad::g_core[i]',
                        posw='''data.cores[i].x = b.x;
        data.cores[i].y = b.y;
        data.cores[i].z = b.z;''')
V['p9_forloop'] = fill(REC_MVEC, EX_SA % 'mPad::g_core[i]').replace(
    '''    int i = 0;
    do {''', '''    for (int i = 0; i < 4; ++i) {''').replace(
    '    } while (++i <= 3);\n', '    }\n')
V['p10_cccache'] = fill(REC_MVEC, EX_CC % 'cc', ccspell='cc',
                        distw='data.cores[i].dist = cc->getDpdDistance();')


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
        return name, None, 'BUILDFAIL ' + str(log).strip().splitlines()[-1][:90]
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
    if re.search(r'addi r\d+, r1, 0x50', text):
        sig.append('FIXBASE')
    matched, msg = harness.diff_fn(TARGET, txt, FN)[:2]
    if matched:
        return name, 0, 'MATCH ' + ' '.join(sig)
    nd = sum(1 for k in range(max(len(want), len(got)))
             if (want[k] if k < len(want) else None) !=
                (got[k] if k < len(got) else None))
    return name, nd, '%dw %s %s' % (len(got), str(msg).splitlines()[0][:30],
                                    ' '.join(sig))


if __name__ == '__main__':
    rows = [gen(n, s) for n, s in sorted(V.items())]
    rows.sort(key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999))
    for n, d, m in rows:
        print('%-16s %-6s %s' % (n, d, m))
