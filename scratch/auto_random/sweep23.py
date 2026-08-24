"""Sweep 23: all-POD local Seed struct (no ctors -> no __construct_array).

q02's inflation came from RandRec having mVec3_c/mVec2_c members (__construct_array,
~12 words). An all-float RandRec keeps the identical memory layout while letting
seed.cores[i] index a true member array -- hoping for base+IV addressing:
  addi rX, r1, 0x50 ; add rX, rX, rIV ; rIV += 0x18
  lis/addi g_core base ; lwzx ; rIV += 4
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw23')
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

typedef struct {
    float x, y, z;
    float rx, ry;
    float dist;
} RandRec;

typedef struct {
    u64 time;
    char nick[0x18];
    RandRec cores[4];
} Seed;

} // namespace

u32 dRandom_c::calcMachineRandom() {
'''

EX_CTOR = '''        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);'''

RAW_FILL = '''        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        seed.cores[i].rx = raw.x;
        seed.cores[i].ry = raw.y;'''

RAW_DIRECT = '''        seed.cores[i].rx = rp.x;
        seed.cores[i].ry = rp.y;'''


def mk(recbind, raw, ex=EX_CTOR):
    return HEAD + '''    Seed seed;
    memset(&seed, 0, sizeof(Seed));
    seed.time = OSGetTime();
    SCGetOwnerNickName(seed.nick);

    int i = 0;
    do {
@BIND@
@EX@
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        seed.cores[i].x = b.x;
        seed.cores[i].y = b.y;
        seed.cores[i].z = b.z;

@RAW@
        seed.cores[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(&seed, sizeof(Seed));
}
'''.replace('@BIND@', recbind).replace('@EX@', ex).replace('@RAW@', raw)


REFBIND = '        RandRec &rec = seed.cores[i];'
PTRBIND = '        RandRec *rec = &seed.cores[i];'


def with_rec_ref(src):
    return src


V = {
    't1_fullidx': mk('        (void)0;', RAW_FILL),
    't2_refbind': mk(REFBIND.replace(
        'rec', 'rec') , RAW_FILL),
    't3_ptrbind': mk(PTRBIND, RAW_FILL),
    't4_rawdirect': mk('        (void)0;', RAW_DIRECT),
}

# fix t2/t3 to actually use rec
def use_rec(name, structform='seed.cores[i]'):
    s = V[name]
    return (s.replace('seed.cores[i].x = b.x;', 'rec.x = b.x;')
             .replace('seed.cores[i].y = b.y;', 'rec.y = b.y;')
             .replace('seed.cores[i].z = b.z;', 'rec.z = b.z;')
             .replace('seed.cores[i].rx = raw.x;', 'rec.rx = raw.x;')
             .replace('seed.cores[i].ry = raw.y;', 'rec.ry = raw.y;')
             .replace('seed.cores[i].rx = rp.x;', 'rec.rx = rp.x;')
             .replace('seed.cores[i].ry = rp.y;', 'rec.ry = rp.y;')
             .replace('seed.cores[i].dist =', 'rec.dist ='))


V['t2_refbind'] = use_rec('t2_refbind').replace('rec.', 'rec.')
V['t3_ptrbind'] = use_rec('t3_ptrbind').replace('rec.', 'rec->')


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
        return name, None, 'BUILDFAIL ' + str(log).strip().splitlines()[-1][:110]
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
    if re.search(r'mulli', text):
        sig.append('MULLI!')
    if re.search(r'\baddi\s+r\d+, r1, 0x50', text):
        sig.append('REB50')
    if '__construct_array' in full:
        sig.append('CTORARR!')
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
