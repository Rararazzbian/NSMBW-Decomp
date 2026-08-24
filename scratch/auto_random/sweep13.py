"""Sweep 13: refined skeleton matching observed target schedule.

Axes:
  dpos  = where `float *dst` is declared (early | late)
  rawf  = how the dead vec2 copy is made (set | cctor | none)
  d34   = whether dst[3]/dst[4] read rp or raw
  ab    = declaration order of the mVec3_c pair (ab | ba)
"""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw13')
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

u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);
    int i = 0;
    do {
'''

TAIL = '''    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
'''


def body(dpos, rawf, d34, ab):
    L = []
    L.append('EGG::CoreController *cc = mPad::g_core[i];')
    L.append('const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(\n'
             '            reinterpret_cast<const char *>(cc) + 0x24);')
    if ab == 'ab':
        L.append('mVec3_c a(pos);')
        L.append('mVec3_c b(a);')
    else:
        L.append('mVec3_c b(pos);')
        L.append('mVec3_c a(b);')
    L.append('EGG::Vector2f rp = cc->getDpdRawPos();')
    if dpos == 'early':
        L.insert(1, 'float *dst = reinterpret_cast<float *>(buf + 0x20) + i * 6;')
    else:
        L.append('float *dst = reinterpret_cast<float *>(buf + 0x20) + i * 6;')

    src3 = ['a.x', 'a.y', 'a.z'] if ab == 'ba' else ['b.x', 'b.y', 'b.z']
    other = ['b', 'a'] if ab == 'ba' else ['a', 'b']
    # both objects exist; write dst[0..2] from the LIVE one (read-back object).
    # For ab: b is read back. For ba: a is read back.
    L.append('dst[0] = %s;' % src3[0])
    L.append('dst[1] = %s;' % src3[1])
    L.append('dst[2] = %s;' % src3[2])

    if rawf == 'set':
        L.append('mVec2_c raw;')
        L.append('raw.set(rp.x, rp.y);')
    elif rawf == 'cctor':
        L.append('mVec2_c raw(*reinterpret_cast<const mVec2_c *>(&rp));')
    elif rawf == 'setfirst':
        L.append('mVec2_c raw;')
        L.append('raw.set(rp.x, rp.y);')
        L.append('dst[3] = raw.x;'.replace('PLACE', ''))
    if d34 == 'rp':
        L.append('dst[3] = rp.x;' if rawf != 'setfirst' else None)
        L.append('dst[4] = rp.y;')
    else:
        if rawf != 'setfirst':
            pass
        if not any(x == 'dst[3] = raw.x;' for x in L):
            L.append('dst[3] = raw.x;')
        L.append('dst[4] = raw.y;')
    L.append('dst[5] = mPad::g_core[i]->getDpdDistance();')
    return '\n        '.join(x for x in L if x)


def gen(name, **kw):
    src = HEAD + body(**kw) + TAIL
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
    if matched:
        return name, 0, 'MATCH'
    want = harness.extract(TARGET, FN)
    got = harness.extract(txt, FN) or []
    ndiff = sum(1 for i in range(max(len(want), len(got)))
                if (want[i] if i < len(want) else '<none>') !=
                   (got[i] if i < len(got) else '<none>'))
    first = msg.splitlines()[1].strip()[:44]
    return name, ndiff, '%dw %s' % (len(got), first)


if __name__ == '__main__':
    rows = []
    for dpos in ['early', 'late']:
        for rawf in ['set', 'cctor']:
            for d34 in ['rp', 'raw']:
                for ab in ['ab', 'ba']:
                    name = 'e_%s_%s_%s_%s' % (dpos, rawf, d34, ab)
                    rows.append(gen(name, dpos=dpos, rawf=rawf, d34=d34, ab=ab))
    for n, d, m in sorted(rows, key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999)):
        print('%-30s %-6s %s' % (n, d, m))
