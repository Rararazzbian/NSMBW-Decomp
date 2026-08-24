"""Sweep 20: structural shape from manual decode of the 62w target.

Target facts being hunted:
  frame 0xd0, _savegpr_27            (five callee-saved GPRs live across calls)
  lis/addi g_core base -> r29; byte-IV r30 += 4; lwzx r4, r29, r30
  per-iter rec address rebuild: addi r27, r1, 0x50 ; add r27, r27, r31 (r31 += 0x18)
  counter r28, ++i before dist store, cmpwi r28, 3 ; ble
  mVec3_c copies: FIRST store triple -> 0x18(a), SECOND -> 0x24(b),
                  record fields READ BACK from b (0x24/28/2c), loads z,y,x order
  raw section: st rp.x->rec.raw.x ; reload cc ; st raw.x/raw.y locals ;
               st rp.y->rec.raw.y
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw20')
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
@REC@
};

struct SeedData {
    u64 time;
    char nick[0x18];
    RandRec cores[4];
};

} // namespace

u32 dRandom_c::calcMachineRandom() {
'''

REC_VEC = '''    mVec3_c pos;
    mVec2_c raw;
    float dist;'''

REC_FLT = '''    float x, y, z;
    mVec2_c raw;
    float dist;'''

E1 = '''        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);'''

E2 = '''        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a;
        mVec3_c b;
        a = r;
        b = a;'''

E3 = '''        const mVec3_c p =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(p);
        mVec3_c b(a);'''

P1 = '        data.cores[i].pos = b;'

P2 = '''        data.cores[i].x = b.x;
        data.cores[i].y = b.y;
        data.cores[i].z = b.z;'''

W1 = '''        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        data.cores[i].raw = raw;'''

W2 = '''        data.cores[i].raw.x = rp.x;
        data.cores[i].raw.y = rp.y;'''

W3 = '''        mVec2_c raw(rp);
        data.cores[i].raw = raw;'''

W4 = '''        data.cores[i].raw.x = rp.x;
        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        data.cores[i].raw.y = rp.y;'''

TIME1 = '    data.time = OSGetTime();'
TIME2 = '    *(s64 *)&data.time = OSGetTime();'


def body(extract, posw, raww, timeform):
    s = HEAD.replace('@REC@', REC_FLT if posw is P2 else REC_VEC)
    s += '''    SeedData data;
    memset(&data, 0, sizeof(SeedData));
@TIME@
    SCGetOwnerNickName(data.nick);

    int i = 0;
    do {
@EX@
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

@POSW@

@RAW@
        data.cores[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(&data, sizeof(SeedData));
}
'''
    return (s.replace('@TIME@', timeform).replace('@EX@', extract)
             .replace('@POSW@', posw).replace('@RAW@', raww))


V = {
    'q01_E1_P1_W1': body(E1, P1, W1, TIME1),
    'q02_E1_P2_W1': body(E1, P2, W1, TIME1),
    'q03_E2_P1_W1': body(E2, P1, W1, TIME1),
    'q04_E2_P2_W1': body(E2, P2, W1, TIME1),
    'q05_E1_P2_W2': body(E1, P2, W2, TIME1),
    'q06_E1_P2_W3': body(E1, P2, W3, TIME1),
    'q07_E1_P2_W4': body(E1, P2, W4, TIME1),
    'q08_E3_P2_W1': body(E3, P2, W1, TIME1),
    'q09_E1_P2_W1_T2': body(E1, P2, W1, TIME2),
    'q10_E2_P2_W4': body(E2, P2, W4, TIME1),
    'q11_E1_P1_W3': body(E1, P1, W3, TIME1),
    'q12_E2_P2_W2': body(E2, P2, W2, TIME1),
}


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
    if '_savegpr_27' in full:
        sig.append('sv27')
    if re.search(r'lwzx r\d+, r\d+, r\d+', text):
        sig.append('LWZX')
    if 'addi r27, r1, 0x50' in text:
        sig.append('R27REB')
    # first mVec3-copy store slot: 0x18 (a first) or 0x24 (b first)
    m = re.search(r'stfs f\d+, 0x(18|24)\(r1\)', text)
    if m:
        sig.append('fst' + m.group(1))
    matched, msg = harness.diff_fn(TARGET, txt, FN)[:2]
    if matched:
        return name, 0, 'MATCH ' + ' '.join(sig)
    nd = sum(1 for k in range(max(len(want), len(got)))
             if (want[k] if k < len(want) else None) !=
                (got[k] if k < len(got) else None))
    return name, nd, '%dw %s %s' % (len(got), str(msg).splitlines()[0][:34],
                                    ' '.join(sig))


if __name__ == '__main__':
    rows = [gen(n, s) for n, s in sorted(V.items())]
    rows.sort(key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999))
    for n, d, m in rows:
        print('%-16s %-6s %s' % (n, d, m))
