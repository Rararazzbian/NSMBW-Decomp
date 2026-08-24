"""Sweep 27 (kitchen sink): declare locals in TARGET slot order [rp, raw, a, b],
varying scope and extraction form. Last sweep before park decision.
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw27')
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
    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

'''

TAIL = '''    return OSCalcCRC32(buf, sizeof(buf));
}
'''

EXTRACT_REF = '''        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);
'''

MID = '''        rp = mPad::g_core[i]->getDpdRawPos();

        rec.x = b.x;
        rec.y = b.y;
        rec.z = b.z;

        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;

        rec.dist = mPad::g_core[i]->getDpdDistance();
'''

# k1: rp, raw declared at top of loop body
K1 = HEAD + '''    int i = 0;
    do {
        RandRec &rec = reinterpret_cast<RandRec *>(buf + 0x20)[i];
        EGG::Vector2f rp;
        mVec2_c raw;
''' + EXTRACT_REF + MID + '''    } while (++i <= 3);

''' + TAIL

# k2: rp, raw declared at function scope before the loop
K2 = HEAD + '''    EGG::Vector2f rp;
    mVec2_c raw;
    int i = 0;
    do {
        RandRec &rec = reinterpret_cast<RandRec *>(buf + 0x20)[i];
''' + EXTRACT_REF + MID + '''    } while (++i <= 3);

''' + TAIL

# k3: function-scope order [rp, raw] plus a, b ALSO at function scope
K3 = HEAD + '''    EGG::Vector2f rp;
    mVec2_c raw;
    int i = 0;
    do {
        RandRec &rec = reinterpret_cast<RandRec *>(buf + 0x20)[i];
        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
''' + MID.replace('''        rp = ''', '''        a = r;
        b = a;
        rp = ''').replace('rec.x = b.x;', 'rec.x = b.x;') + '''    } while (++i <= 3);

''' + TAIL.replace('}', '}')
K3 = K3.replace('    return OSCalcCRC32', '''    mVec3_c a;
    mVec3_c b;
    return OSCalcCRC32''')

V = {'k1_bodyscope': K1, 'k2_funcscope': K2, 'k3_allfuncscope': K3}


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
    m = re.search(r'stfs f\d+, 0x(18|24)\(r1\)', text)
    if m:
        sig.append('fst' + m.group(1))
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
        print('%-20s %-6s %s' % (n, d, m))
