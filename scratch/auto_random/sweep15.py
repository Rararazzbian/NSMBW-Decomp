"""Sweep 15b: REAL variant matrix -- no-cc-cache x dst spelling x raw feed."""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw15')
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
'''

TAIL = '''    return OSCalcCRC32(buf, 0x80);
}
'''


def gen_body(dstform='floatmul', C=False, D=False, B=False):
    """dstform: floatmul | charmul | idxmul | recptr
       C: dst declared first in body
       D: dst[3]/dst[4] fed from raw
       B: raw written via set()"""
    L = []
    dstexpr_float = 'reinterpret_cast<float *>(buf + 0x20) + i * 6'
    dstexpr_char = 'reinterpret_cast<float *>(buf + 0x20 + i * 0x18)'
    base_expr = 'reinterpret_cast<float *>(buf + 0x20)'
    recexpr = '(reinterpret_cast<RandRec *>(&buf[0x20]) + i)'
    dstdesc = {'floatmul': dstexpr_float, 'charmul': dstexpr_char}.get(dstform)

    def dst_lines():
        if dstform == 'recptr':
            return ['RandRec &rc = *' + recexpr + ';',
                    'rc.x = b.x;',
                    'rc.y = b.y;',
                    'rc.z = b.z;']
        if dstform == 'idxmul':
            return ['%s[i * 6 + 0] = b.x;' % base_expr,
                    '%s[i * 6 + 1] = b.y;' % base_expr,
                    '%s[i * 6 + 2] = b.z;' % base_expr]
        return ['dst[0] = b.x;', 'dst[1] = b.y;', 'dst[2] = b.z;']

    if C:
        L.append('float *dst = %s;' % dstdesc)
    L.append('const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(\n'
             '        reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);')
    L.append('mVec3_c a(pos);')
    L.append('mVec3_c b(a);')
    L.append('EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();')
    if not C:
        L.append('float *dst = %s;' % dstdesc)
    L.extend(dst_lines())
    L.append('mVec2_c raw;')
    if D:
        if B:
            L.append('raw.set(rp.x, rp.y);')
        else:
            L.append('raw.x = rp.x;')
            L.append('raw.y = rp.y;')
        if dstform in ('recptr',):
            L.append('rc.raw = raw;')
        elif dstform == 'idxmul':
            L.append('%s[i * 6 + 3] = raw.x;' % base_expr)
            L.append('%s[i * 6 + 4] = raw.y;' % base_expr)
        else:
            L.append('dst[3] = raw.x;')
            L.append('dst[4] = raw.y;')
    else:
        if dstform in ('recptr',):
            L.append('rc.raw.x = rp.x;')
            L.append('rc.raw.y = rp.y;')
        elif dstform == 'idxmul':
            L.append('%s[i * 6 + 3] = rp.x;' % base_expr)
            L.append('%s[i * 6 + 4] = rp.y;' % base_expr)
        else:
            L.append('dst[3] = rp.x;')
            L.append('dst[4] = rp.y;')
        L.append('(void)raw;')
    if dstform == 'recptr':
        L.append('rc.dist = mPad::g_core[i]->getDpdDistance();')
    elif dstform == 'idxmul':
        L.append('%s[i * 6 + 5] = mPad::g_core[i]->getDpdDistance();' % base_expr)
    else:
        L.append('dst[5] = mPad::g_core[i]->getDpdDistance();')
    inner = '\n        '.join(L)
    return ('    int i = 0;\n    do {\n        ' + inner +
            '\n    } while (++i <= 3);\n')


def gen(name, **kw):
    src = HEAD + gen_body(**kw) + TAIL
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
    rows = [
        gen('v1_floatmul', dstform='floatmul'),
        gen('v2_floatmul_C', dstform='floatmul', C=True),
        gen('v3_charmul', dstform='charmul'),
        gen('v4_idxmul', dstform='idxmul'),
        gen('v5_recptr', dstform='recptr'),
        gen('v6_floatmul_D', dstform='floatmul', D=True),
        gen('v7_recptr_D', dstform='recptr', D=True),
        gen('v8_charmul_D', dstform='charmul', D=True),
    ]
    rows.sort(key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999))
    for n, d, m in rows:
        print('%-16s %-6s %s' % (n, d, m))
