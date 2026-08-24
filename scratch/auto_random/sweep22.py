"""Sweep 22: canonical [i] indexing over an explicit &buf[0x20] base.

Target lowering being hunted:
  addi r27, r1, 0x50 ; add r27, r27, r31 ; r31 += 0x18   (rec IV from 0)
  lwzx r4, r29, r30 ; r30 += 4                           (g_core base + IV from 0)
  r28 counter, ++i <= 3
r01 proved the frame-base rebuild appears when the cast is INSIDE the loop;
the residual mulli came from spelling [i + 4]. Spell [i] on a buf+0x20 base.
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw22')
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

RAW_FILL = '''        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;'''


def mk(bindline, ex, raw, fields='b', earlydecl=False):
    decl = '''    EGG::Vector2f rp;
    mVec2_c raw;
''' if earlydecl else ''
    rpassign = ('        rp = mPad::g_core[i]->getDpdRawPos();' if earlydecl
                else '        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();')
    rawbody = ('''        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;''' if earlydecl else raw)
    return HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

''' + decl + '''    int i = 0;
    do {
@BIND@
@EX@
@RP@

        rec.@F1@ = @V@.x;
        rec.@F2@ = @V@.y;
        rec.@F3@ = @V@.z;

@RAWBODY@
        rec.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
'''.replace('@BIND@', bindline).replace('@EX@', ex).replace('@RP@', rpassign) \
   .replace('@RAWBODY@', rawbody).replace('@F1@', 'x').replace('@F2@', 'y') \
   .replace('@F3@', 'z').replace('@V@', fields)


BIND_I = '        RandRec &rec = reinterpret_cast<RandRec *>(buf + 0x20)[i];'
BIND_ARRREF = ('        RandRec (&recs)[4] =\n'
               '            *reinterpret_cast<RandRec (*)[4]>(&buf[0x20]);')

V = {
    's1_i': mk(BIND_I, EX_CTOR, RAW_FILL),
    's2_early': mk(BIND_I, EX_CTOR, RAW_FILL, earlydecl=True),
    's3_afields': mk(BIND_I, EX_CTOR, RAW_FILL, fields='a'),
    's4_arrref': mk('        RandRec &rec = recs[i];'.replace(
        'recs', 'RECS_PLACE'), EX_CTOR, RAW_FILL),
}
V['s4_arrref'] = V['s4_arrref'].replace('RECS_PLACE', 'recs')
# s4 needs the arrref binding line instead
V['s4_arrref'] = V['s4_arrref'].replace(BIND_I, BIND_ARRREF)


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
    if re.search(r'addi r\d+, r1, 0x50', text):
        sig.append('REB50')
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
