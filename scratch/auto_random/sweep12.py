"""Sweep 12: structural variants for calcMachineRandom (fresh session).

Axes:
  dst   = add | charptr | rec
  core  = twice | local
  v3    = pair | arr2 | scalar
  vec2  = assign | cctor | direct
"""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw12')
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


def body(dst, core, v3, vec2, loop):
    if dst == 'add':
        dstdecl = 'float *dst = reinterpret_cast<float *>(buf + 0x20) + i * 6;'
        w = ['dst[0]', 'dst[1]', 'dst[2]', 'dst[3]', 'dst[4]', 'dst[5]']
    elif dst == 'charptr':
        dstdecl = 'float *dst = reinterpret_cast<float *>(&buf[0x20 + i * 0x18]);'
        w = ['dst[0]', 'dst[1]', 'dst[2]', 'dst[3]', 'dst[4]', 'dst[5]']
    elif dst == 'rec':
        dstdecl = ('struct Rec { float v[6]; };\n'
                   '        Rec *rec = reinterpret_cast<Rec *>(buf + 0x20) + i;')
        w = ['rec->v[0]', 'rec->v[1]', 'rec->v[2]',
             'rec->v[3]', 'rec->v[4]', 'rec->v[5]']
    else:
        raise ValueError(dst)

    if core == 'twice':
        pre = ''
        ccexpr = 'mPad::g_core[i]'
        reload_dist = '%s->getDpdDistance()' % ccexpr
    else:
        pre = 'EGG::CoreController *cc = mPad::g_core[i];\n        '
        ccexpr = 'cc'
        reload_dist = 'cc->getDpdDistance()'

    if v3 == 'pair':
        v3code = (
            'const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(\n'
            '            reinterpret_cast<const char *>(%s) + 0x24);\n'
            '        mVec3_c a(pos);\n'
            '        mVec3_c b(a);\n' % ccexpr)
        v3src = ['b.x', 'b.y', 'b.z']
    elif v3 == 'arr2':
        v3code = (
            'const f32 *fp = reinterpret_cast<const f32 *>(\n'
            '            reinterpret_cast<const char *>(%s) + 0x24);\n'
            '        mVec3_c a(fp);\n'
            '        mVec3_c b(fp);\n' % ccexpr)
        v3src = ['b.x', 'b.y', 'b.z']
    elif v3 == 'scalar':
        v3code = (
            'float ax = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(%s) + 0x24);\n'
            '        float ay = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(%s) + 0x28);\n'
            '        float az = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(%s) + 0x2c);\n'
            % (ccexpr, ccexpr, ccexpr))
        v3src = ['ax', 'ay', 'az']
    else:
        raise ValueError(v3)

    if vec2 == 'assign':
        v2code = ('EGG::Vector2f rp = %s->getDpdRawPos();\n'
                  '        mVec2_c raw;\n'
                  '        raw.set(rp.x, rp.y);\n' % ccexpr)
        v2src = ['raw.x', 'raw.y']
    elif vec2 == 'cctor':
        v2code = ('EGG::Vector2f rp = %s->getDpdRawPos();\n'
                  '        mVec2_c raw(*reinterpret_cast<const mVec2_c *>(&rp));\n' % ccexpr)
        v2src = ['raw.x', 'raw.y']
    elif vec2 == 'direct':
        v2code = 'EGG::Vector2f rp = %s->getDpdRawPos();\n' % ccexpr
        v2src = ['rp.x', 'rp.y']
    else:
        raise ValueError(vec2)

    stmts = [
        dstdecl,
        pre + v3code.rstrip(),
        '%s = %s;' % (w[0], v3src[0]),
        '%s = %s;' % (w[1], v3src[1]),
        '%s = %s;' % (w[2], v3src[2]),
        v2code.rstrip(),
        '%s = %s;' % (w[3], v2src[0]),
        '%s = %s;' % (w[4], v2src[1]),
        '%s = %s;' % (w[5], reload_dist),
    ]
    inner = '\n        '.join(stmts)

    if loop == 'do':
        return ('    int i = 0;\n'
                '    do {\n'
                '        ' + inner + '\n'
                '    } while (++i <= 3);\n')
    else:
        return ('    for (int i = 0; i < 4; i++) {\n'
                '        ' + inner + '\n'
                '    }\n')


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
        return name, None, 'BUILDFAIL ' + str(log).strip().splitlines()[-1][:80]
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
    first = msg.splitlines()[1].strip()[:46]
    return name, ndiff, '%dw %s' % (len(got), first)


if __name__ == '__main__':
    rows = []
    for dst in ['add', 'charptr', 'rec']:
        for core in ['twice', 'local']:
            for v3 in ['pair', 'arr2', 'scalar']:
                for vec2 in ['assign', 'cctor', 'direct']:
                    name = 'd_%s_%s_%s_%s' % (dst, core, v3, vec2)
                    rows.append(gen(name, dst=dst, core=core, v3=v3,
                                    vec2=vec2, loop='do'))
    for n, d, m in sorted(rows, key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999)):
        print('%-42s %-6s %s' % (n, d, m))
