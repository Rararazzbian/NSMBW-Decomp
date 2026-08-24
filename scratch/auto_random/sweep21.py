"""Sweep 21: per-iteration rec binding hypothesis.

Target signature (62w):
  addi r27, r1, 0x50 ; add r27, r27, r31   INSIDE loop  <- rec addr rebuilt
  r31 += 0x18 (rec IV), r30 += 4 (core IV), lwzx r4, r29, r30 (unfused g_core)
  _savegpr_27, frame 0xd0
Hypothesis: the record is accessed through a binding made INSIDE the loop body,
so its address cannot be hoisted/cursor-advanced.
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw21')
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
    mVec2_c raw;
    float dist;
} RandRec;

} // namespace

u32 dRandom_c::calcMachineRandom() {
'''

EX_CTOR = '''        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);'''

EX_CTOR_DIRECT = '''        mVec3_c a(*reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24));
        mVec3_c b(a);'''

RAW_FILL = '''        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;'''

RAW_CTOR = '''        mVec2_c raw(rp);
        rec.raw = raw;'''

RAW_DIRECT = '''        rec.raw.x = rp.x;
        rec.raw.y = rp.y;'''


def v1(recbind):
    return HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
@BIND@
@EX@
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        rec.x = b.x;
        rec.y = b.y;
        rec.z = b.z;

@RAW@
        rec.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
'''.replace('@BIND@', recbind).replace('@EX@', EX_CTOR).replace('@RAW@', RAW_FILL)


def v2(recbind, ex):
    return HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
@BIND@
@EX@
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        rec.x = b.x;
        rec.y = b.y;
        rec.z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;

        rec.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
'''.replace('@BIND@', recbind).replace('@EX@', ex)


BIND_REF = '        RandRec &rec = reinterpret_cast<RandRec *>(buf)[i + 4];'
BIND_IDX = ('        RandRec &rec =\n'
            '            reinterpret_cast<RandRec *>(buf)[i + 4];')
BIND_PTR = '        RandRec *rec = &reinterpret_cast<RandRec *>(buf)[i + 4];'

V = {
    'r01_ref': v1(BIND_REF),
    'r02_ptr': v1(BIND_PTR.replace('rec =', 'rec =').replace('*rec', '*rec')),
    'r03_directctor': v2(BIND_REF, EX_CTOR_DIRECT),
    'r04_rawctor': v1(BIND_REF).replace(RAW_FILL, RAW_CTOR),
    'r05_rawdirect': v1(BIND_REF).replace(RAW_FILL, RAW_DIRECT),
}


def fix_r02():
    # pointer form: rewrite field writes through the pointer
    s = V['r02_ptr']
    return (s.replace('rec.x = b.x;', 'rec->x = b.x;')
             .replace('rec.y = b.y;', 'rec->y = b.y;')
             .replace('rec.z = b.z;', 'rec->z = b.z;')
             .replace('rec.raw = raw;', 'rec->raw = raw;')
             .replace('rec.raw.x = rp.x;', 'rec->raw.x = rp.x;')
             .replace('rec.raw.y = rp.y;', 'rec->raw.y = rp.y;')
             .replace('rec.dist =', 'rec->dist ='))


V['r02_ptr'] = fix_r02()


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
    if re.search(r'addi r\d+, r1, 0x50', text):
        sig.append('REB50')
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
