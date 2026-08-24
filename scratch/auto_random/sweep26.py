"""Sweep 26: local slot geometry -- raw/rp introduction order.

Mine allocates raw@0x8, rp@0x10 (reverse of introduction). Target has
rp@0x8, raw@0x10. If allocation is reverse-introduction-order, declaring
raw BEFORE the rawPos call should flip the pair to target layout, possibly
also flipping the a/b store order with it.
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw26')
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

EXTRACT = '''        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);
'''

CALL = '        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();'

FIELDS = '''        rec.x = b.x;
        rec.y = b.y;
        rec.z = b.z;
'''


def mk(pre_call, post_call):
    late_raw = '' if 'mVec2_c raw;' in pre_call else '        mVec2_c raw;\n'
    return HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
        RandRec &rec = reinterpret_cast<RandRec *>(buf + 0x20)[i];
@PRE@
@EXTRACT@
@POST@
@FIELDS@
@LATERAW@        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;

        rec.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
'''.replace('@PRE@', pre_call).replace('@EXTRACT@', EXTRACT) \
   .replace('@POST@', post_call).replace('@FIELDS@', FIELDS) \
   .replace('@LATERAW@', late_raw)


V = {
    # baseline: nothing before extract, call after (current shape)
    'w0_baseline': mk('', CALL),
    # raw declared right before the call
    'w1_raw_befcall': mk('        mVec2_c raw;', CALL),
    # raw declared at very top of body
    'w2_raw_top': mk('        mVec2_c raw;', ''),
    # raw declared before extract, call stays after
    'w3_raw_preextract': mk('', '').replace(
        '@EXTRACT@\n', '        mVec2_c raw;\n' + EXTRACT + CALL + '\n'),
}

# fix w3 template leftovers
V['w3_raw_preextract'] = HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
        RandRec &rec = reinterpret_cast<RandRec *>(buf + 0x20)[i];
        mVec2_c raw;
''' + EXTRACT + CALL + '''

''' + FIELDS + '''
        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;

        rec.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
'''


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
    sig = []
    # locate rp slot (first lfs from a small frame offset after the rawPos call)
    callpos = next((k for k, l in enumerate(got) if 'getDpdRawPos' in l), None)
    if callpos is not None:
        for k in range(callpos + 1, min(callpos + 6, len(got))):
            m = re.match(r'\s*lfs f\d, (0x[0-9a-f]+)\(r1\)', got[k])
            if m:
                sig.append('rp@' + m.group(1))
                break
    m = re.search(r'stfs f\d+, 0x(18|24)\(r1\)', text)
    if m:
        sig.append('fst' + m.group(1))
    if '_savegpr_27' in open(txt).read():
        sig.append('sv27')
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
