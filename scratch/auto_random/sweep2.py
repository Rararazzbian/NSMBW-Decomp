"""Sweep 2 for calcMachineRandom: induction structure and declaration matrices."""
import os
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/scratch/auto_random')
import build
import harness

BASE = build.BASE
FN = build.FN

PRELUDE = '''#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

{RECDEF}

u32 dRandom_c::calcMachineRandom() {{
'''

REC_PLAIN = '''namespace {
struct Rec { float x, y, z, rx, ry, dist; };
}
'''

REC_VEC = '''namespace {
struct Rec { mVec3_c pos; float rx, ry, dist; };
}
'''


def mk(name, recdef, decls, loophead, loopbody, looptail):
    src = PRELUDE.format(RECDEF=recdef)
    src += decls + '\n'
    src += '    memset(buf, 0, 0x80);\n'
    src += '    *(s64 *)buf = OSGetTime();\n'
    src += '    SCGetOwnerNickName(&buf[8]);\n\n'
    src += loophead + ' {\n' + loopbody + '    }\n' + looptail + '\n'
    src += '    return OSCalcCRC32(buf, 0x80);\n}\n'
    p = os.path.join(BASE, 's2_%s.cpp' % name)
    open(p, 'w').write(src)
    return name, p


POS_CAST = ('        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(\n'
            '            reinterpret_cast<const char *>({CC}) + 0x24);\n')

VARIANTS = []

# family A: plain Rec, two g_core[i] evals, ref-local vs direct
for dname, decls in [
    ('decl_ri', '    Rec recs[4];\n    char buf[0x80];\n    int i = 0;'),
    ('decl_ir', '    int i = 0;\n    Rec recs[4];\n    char buf[0x80];'),
    ('decl_rin', '    Rec recs[4];\n    int i = 0;\n    char buf[0x80];'),
]:
    for lname, head, tail in [
        ('dowhile', '    do', ' while (++i < 4);'),
        ('dowhile_le', '    do', ' while (++i <= 3);'),
        ('for', '    for (i = 0; i < 4; i++)', ''),
    ]:
        body = (
            POS_CAST.format(CC='mPad::g_core[i]') +
            '        mVec3_c a(pos);\n'
            '        mVec3_c b(a);\n'
            '        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();\n'
            '\n'
            '        recs[i].x = b.x;\n'
            '        recs[i].y = b.y;\n'
            '        recs[i].z = b.z;\n'
            '        recs[i].rx = rp.x;\n'
            '        recs[i].ry = rp.y;\n'
            '        recs[i].dist = mPad::g_core[i]->getDpdDistance();\n')
        VARIANTS.append(mk('A_%s_%s' % (dname, lname), REC_PLAIN, decls, head, body, tail))

# family B: Rec with mVec3_c member, struct assign for the triple
body_b = (
    POS_CAST.format(CC='mPad::g_core[i]') +
    '        mVec3_c a(pos);\n'
    '        mVec3_c b(a);\n'
    '        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();\n'
    '\n'
    '        recs[i].pos = b;\n'
    '        recs[i].rx = rp.x;\n'
    '        recs[i].ry = rp.y;\n'
    '        recs[i].dist = mPad::g_core[i]->getDpdDistance();\n')
VARIANTS.append(mk('B_decl_ri_dowhile', REC_VEC,
                   '    Rec recs[4];\n    char buf[0x80];\n    int i = 0;',
                   '    do', body_b, ' while (++i < 4);'))

# family C: reference local rr = recs[i]
body_c = (
    '        Rec &rr = recs[i];\n' +
    POS_CAST.format(CC='mPad::g_core[i]') +
    '        mVec3_c a(pos);\n'
    '        mVec3_c b(a);\n'
    '        EGG::Vector3f rp = mPad::g_core[i]->getDpdRawPos();\n'
    '\n'
    '        rr.x = b.x;\n'
    '        rr.y = b.y;\n'
    '        rr.z = b.z;\n'
    '        rr.rx = rp.x;\n'
    '        rr.ry = rp.y;\n'
    '        rr.dist = mPad::g_core[i]->getDpdDistance();\n')
VARIANTS.append(mk('C_reflocal_dowhile', REC_PLAIN,
                   '    Rec recs[4];\n    char buf[0x80];\n    int i = 0;',
                   '    do', body_c, ' while (++i < 4);'))

# family D: pre-loop cores pointer, plain indexed
body_d = (
    POS_CAST.format(CC='cores[i]') +
    '        mVec3_c a(pos);\n'
    '        mVec3_c b(a);\n'
    '        EGG::Vector3f rp = cores[i]->getDpdRawPos();\n'
    '\n'
    '        recs[i].x = b.x;\n'
    '        recs[i].y = b.y;\n'
    '        recs[i].z = b.z;\n'
    '        recs[i].rx = rp.x;\n'
    '        recs[i].ry = rp.y;\n'
    '        recs[i].dist = cores[i]->getDpdDistance();\n')
VARIANTS.append(mk('D_coresptr_dowhile', REC_PLAIN,
                   '    Rec recs[4];\n    char buf[0x80];\n'
                   '    EGG::CoreController *const *cores = mPad::g_core;\n    int i = 0;',
                   '    do', body_d, ' while (++i < 4);'))


def score(txt):
    t = harness.extract(build.TARGET, FN) or []
    d = harness.extract(txt, FN) or []
    tc = [l.split('*/', 1)[-1].strip() for l in t]
    dc = [l.split('*/', 1)[-1].strip() for l in d]
    diff = sum(1 for i in range(max(len(tc), len(dc)))
               if tc[i:i + 1] != dc[i:i + 1])
    return '%dw/%dw difflines=%d' % (len(d), len(t), diff)


if __name__ == '__main__':
    res = []
    for name, p in VARIANTS:
        ok, log = build.build(p, label='s2_' + name)
        if not ok:
            res.append((99999, name, 'COMPILE FAIL'))
            continue
        txt = os.path.join(BASE, 's2_%s.txt' % name)
        sc = score(txt)
        n = int(sc.split('difflines=')[1])
        res.append((n, name, sc))
    for n, name, msg in sorted(res):
        print('%-6d %-28s %s' % (n, name, msg))
