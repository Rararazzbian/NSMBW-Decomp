"""Sweep 16: attack the three structural deltas.

  D1: g_core access form  -- carried cursor (mine) vs fixed base + byte IV
                             + indexed lwzx (target)
  D2: dst form            -- stfsux update (mine) vs addi/add rebuild +
                             displacement stores (target)
  D3: raw local           -- absent (mine) vs dead-but-present stores in
                             loop (target)

Levers tested here:
  recptr : file-local RandRec struct, recs[i].field spelling
  assign : mVec3_c via copy-ASSIGN not copy-ctor
  sink   : raw passed to an empty inline function (inlined-away call keeps
           the stores alive, cf. AGENT_CONTEXT PlayerIconSet note)
  refslot: bind EGG::CoreController *& reference to the array SLOT
"""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw16')
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

'''


def gen_body(sink=False, assign=False, refslot=False):
    L = []
    L.append('const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(\n'
             '        reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);')
    if assign:
        L.append('mVec3_c a;')
        L.append('a = pos;')
        L.append('mVec3_c b;')
        L.append('b = a;')
    else:
        L.append('mVec3_c a(pos);')
        L.append('mVec3_c b(a);')
    L.append('EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();')
    L.append('RandRec &rc = reinterpret_cast<RandRec *>(&buf[0x20])[i];')
    L.append('rc.x = b.x;')
    L.append('rc.y = b.y;')
    L.append('rc.z = b.z;')
    L.append('mVec2_c raw;')
    L.append('raw.x = rp.x;')
    L.append('raw.y = rp.y;')
    if sink:
        L.append('sink(raw);')
    else:
        L.append('rc.raw = raw;')
    L.append('rc.dist = mPad::g_core[i]->getDpdDistance();')
    inner = '\n        '.join(L)
    return ('    int i = 0;\n    do {\n        ' + inner +
            '\n    } while (++i <= 3);\n')


TAIL = '''    return OSCalcCRC32(buf, 0x80);
}
'''


def gen(name, sink=False, assign=False, refslot=False):
    sinkfn = 'inline void sink(const mVec2_c &) {}\n\n' if sink else ''
    src = HEAD + sinkfn + '} // namespace\n\nu32 dRandom_c::calcMachineRandom() {\n    char buf[0x80];\n    memset(buf, 0, 0x80);\n    *(s64 *)buf = OSGetTime();\n    SCGetOwnerNickName(&buf[8]);\n' + gen_body(sink=sink, assign=assign, refslot=refslot) + TAIL
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
        gen('w1_plain'),
        gen('w2_sink', sink=True),
        gen('w3_assign', assign=True),
        gen('w4_assign_sink', assign=True, sink=True),
    ]
    rows.sort(key=lambda r: (r[1] is None, r[1] if r[1] is not None else 999))
    for n, d, m in rows:
        print('%-16s %-6s %s' % (n, d, m))
