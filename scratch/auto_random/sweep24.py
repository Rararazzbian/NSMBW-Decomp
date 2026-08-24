"""Sweep 24: loop-form / explicit-offset / inlined-helper hunt for
   base+IV addressing (lwzx r4, r29, r30 ; addi rX, r1, 0x50 ; add rX, rX, rIV).
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'sw24')
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

BODY_CORE = '''        RandRec &rec = reinterpret_cast<RandRec *>(buf + 0x20)[i];
        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();

        rec.x = b.x;
        rec.y = b.y;
        rec.z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;

        rec.dist = mPad::g_core[i]->getDpdDistance();
'''

U1 = HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
''' + '    while (i < 4) {\n' + BODY_CORE + '''        ++i;
    }

    return OSCalcCRC32(buf, sizeof(buf));
}
'''

U2 = HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

''' + '    for (int i = 0; i < 4; ++i) {\n' + BODY_CORE + '''    }

    return OSCalcCRC32(buf, sizeof(buf));
}
'''

U3 = HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    do {
''' + BODY_CORE.replace('[i]', '[i]') + '''    } while (i++ < 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
'''

U4 = HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    u32 i = 0;
    do {
''' + BODY_CORE + '''    } while (++i <= 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
'''

U6 = HEAD + '''    char buf[0x80];
    memset(buf, 0, sizeof(buf));
    *(s64 *)buf = OSGetTime();
    SCGetOwnerNickName(&buf[8]);

    int i = 0;
    u32 co = 0;
    u32 ro = 0;
    do {
        EGG::CoreController *cc =
            *(EGG::CoreController **)((char *)&mPad::g_core[0] + co);
        RandRec &rec = *(RandRec *)((char *)buf + 0x20 + ro);
        const mVec3_c &r =
            *reinterpret_cast<const mVec3_c *>(
                reinterpret_cast<const char *>(cc) + 0x24);
        mVec3_c a(r);
        mVec3_c b(a);
        EGG::Vector2f rp = cc->getDpdRawPos();

        rec.x = b.x;
        rec.y = b.y;
        rec.z = b.z;

        mVec2_c raw;
        raw.x = rp.x;
        raw.y = rp.y;
        rec.raw = raw;

        (*(EGG::CoreController **)((char *)&mPad::g_core[0] + co))
            ->getDpdDistance();
        ro += sizeof(RandRec);
        co += 4;
    } while (++i <= 3);

    return OSCalcCRC32(buf, sizeof(buf));
}
'''

U7 = U6.replace('ro += sizeof(RandRec);\n        co += 4;',
                'co += 4;\n        ro += sizeof(RandRec);')
U6b = U6.replace('''        (*(EGG::CoreController **)((char *)&mPad::g_core[0] + co))
            ->getDpdDistance();
''', '''        rec.dist =
            (*(EGG::CoreController **)((char *)&mPad::g_core[0] + co))
                ->getDpdDistance();
''')

V = {'u1_while': U1, 'u2_for': U2, 'u3_postinc': U3, 'u4_u32': U4,
     'u6_litiv': U6b, 'u7_litiv2': U7}


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
