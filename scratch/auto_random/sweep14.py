"""Sweep 14: single-delta ladder from the sw12 prologue-correct baseline.

Baseline shape (d_add_local_pair_assign):
  cc local reused for both calls; dst decl after rp; dst[0..2] before call;
  do-while(++i<=3).

Deltas:
  A: dst[0..2] stores moved AFTER the rp call
  B: distance spelled as fresh mPad::g_core[i]
  C: for-loop instead of do-while
  D: EGG::CoreController **cores = mPad::g_core; local
  E: raw declared before rp
"""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw14')
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

TAIL_D = '''    return OSCalcCRC32(buf, 0x80);
}
'''


def gen_body(A, B, C, D, E):
    L = []
    L.append('int i = 0;' if not C else '')
    if D:
        L.append('EGG::CoreController **cores = mPad::g_core;')
    ccbase = 'cores[i]' if D else 'mPad::g_core[i]'
    distbase = 'cores[i]' if D else 'mPad::g_core[i]'
    L.append('EGG::CoreController *cc = %s;' % ccbase)
    L.append('const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(\n'
             '            reinterpret_cast<const char *>(cc) + 0x24);')
    L.append('mVec3_c a(pos);')
    L.append('mVec3_c b(a);')
    if E:
        L.append('mVec2_c raw;')
    L.append('EGG::Vector2f rp = cc->getDpdRawPos();')
    L.append('float *dst = reinterpret_cast<float *>(buf + 0x20) + i * 6;')
    if not A:
        L.append('dst[0] = b.x;')
        L.append('dst[1] = b.y;')
        L.append('dst[2] = b.z;')
    if not E:
        L.append('mVec2_c raw;')
    L.append('raw.set(rp.x, rp.y);')
    if A:
        L.append('dst[0] = b.x;')
        L.append('dst[1] = b.y;')
        L.append('dst[2] = b.z;')
    L.append('dst[3] = rp.x;')
    L.append('dst[4] = rp.y;')
    if B:
        L.append('dst[5] = %s->getDpdDistance();' % distbase)
    else:
        L.append('dst[5] = cc->getDpdDistance();')
    inner = '\n        '.join(x for x in L if x)
    if C:
        return ('    for (int i = 0; i < 4; i++) {\n'
                '        ' + inner + '\n    }\n')
    return ('    int i = 0;\n    do {\n        ' + inner +
            '\n    } while (++i <= 3);\n')


def gen(name, **kw):
    src = HEAD + gen_body(**kw) + TAIL_D
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
    import itertools
    keys = ['A', 'B', 'C', 'D', 'E']
    for bits in itertools.product([0, 1], repeat=5):
        kw = dict(zip(keys, bits))
        # prune silly combos: E without set-after makes little sense alone; keep all anyway
        name = 'f%s' % ''.join(str(b) for b in bits)
        rows.append(gen(name, **kw))
    for n, d, m in sorted(rows, key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999))[:25]:
        print('%-8s %-6s %s' % (n, d, m))
