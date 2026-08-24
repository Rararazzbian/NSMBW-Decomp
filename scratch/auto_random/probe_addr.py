"""Micro-probes: which source spellings produce
   (a) lwzx rX, rBASE, rIV  (fixed base + byte IV, indexed load) vs
       lwz  rX, 0(rCURSOR)  (carried cursor, displacement load)
   (b) addi+add rebuild + displacement stores vs stfsux update store

Each probe is a self-contained function compiled standalone; we just grep
the disassembly for the addressing forms.
"""
import os
import subprocess
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
OUT = os.path.join(BASE, 'probes')
os.makedirs(OUT, exist_ok=True)

PRE = '''#include <game/bases/d_random.hpp>
#include <MSL/string.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/OS.h>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_vec.hpp>

extern "C" u32 OSCalcCRC32(const void *src, u32 len);
extern "C" BOOL SCGetOwnerNickName(char *nick);

namespace { struct RandRec { float x, y, z; mVec2_c raw; float dist; }; }

'''

PROBES = {
    # p1: plain array indexing both sides
    'p1_index': '''
u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    int i = 0;
    do {
        RandRec &rc = reinterpret_cast<RandRec *>(&buf[0x20])[i];
        EGG::CoreController *cc = mPad::g_core[i];
        rc.x = (float)cc->getKey(0);
        rc.dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
''',
    # p2: float* multiply spelling for records
    'p2_floatmul': '''
u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    int i = 0;
    do {
        float *dst = reinterpret_cast<float *>(&buf[0x20]) + i * 6;
        EGG::CoreController *cc = mPad::g_core[i];
        dst[0] = (float)cc->getKey(0);
        dst[5] = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
''',
    # p3: local copy of the decayed array pointer
    'p3_localbase': '''
u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    EGG::CoreController **cores = mPad::g_core;
    int i = 0;
    do {
        RandRec &rc = reinterpret_cast<RandRec *>(&buf[0x20])[i];
        EGG::CoreController *cc = cores[i];
        rc.x = (float)cc->getKey(0);
        rc.dist = cores[i]->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
''',
    # p4: three textual g_core[i], no cache at all
    'p4_nocache': '''
u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    int i = 0;
    do {
        float *dst = reinterpret_cast<float *>(&buf[0x20]) + i * 6;
        const mVec3_c &pos = *reinterpret_cast<const mVec3_c *>(
            reinterpret_cast<const char *>(mPad::g_core[i]) + 0x24);
        dst[0] = pos.x;
        dst[2] = pos.z;
        EGG::Vector2f rp = mPad::g_core[i]->getDpdRawPos();
        dst[3] = rp.x;
        dst[5] = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
''',
    # p5: pointer-arithmetic spelling for cores
    'p5_ptrarith': '''
u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    int i = 0;
    do {
        RandRec &rc = reinterpret_cast<RandRec *>(&buf[0x20])[i];
        EGG::CoreController *cc = *(mPad::g_core + i);
        rc.x = (float)cc->getKey(0);
        rc.dist = (*(mPad::g_core + i))->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
''',
    # p6: index kept in its own variable, records via recs[i]
    'p6_separateidx': '''
u32 dRandom_c::calcMachineRandom() {
    char buf[0x80];
    memset(buf, 0, 0x80);
    RandRec *const recs = reinterpret_cast<RandRec *>(&buf[0x20]);
    int i = 0;
    do {
        EGG::CoreController *cc = mPad::g_core[i];
        recs[i].x = (float)cc->getKey(0);
        recs[i].dist = mPad::g_core[i]->getDpdDistance();
    } while (++i <= 3);
    return OSCalcCRC32(buf, 0x80);
}
''',
}


def classify(txt):
    body = []
    grab = False
    for ln in open(txt):
        if '.fn calcMachineRandom' in ln:
            grab = True
            continue
        if '.endfn' in ln and grab:
            break
        if grab:
            body.append(ln)
    j = '\n'.join(body)
    core_load = 'lwzx' if 'lwzx' in j else ('disp-lwz' if re.search(
        r'lwz\s+r\d+, 0x0\(r\d+\)', j) else '?')
    dst_form = 'stfsux' if 'stfsux' in j else (
        'rebuild-add' if re.search(r'\badd\s+r\d+, r\d+, r\d+', j) else 'disp')
    n_nv = len(set(re.findall(r'\bstw\s+r(\d+), 0x[0-9a-f]+\(r1\)',
                              j))) + (1 if '_savegpr_' in j else 0)
    frame = re.search(r'stwu\s+r1,\s*-0x([0-9a-f]+)\(r1\)', j)
    return core_load, dst_form, n_nv, frame.group(1) if frame else '?'


import re
if __name__ == '__main__':
    print('%-16s %-9s %-12s %s' % ('probe', 'core', 'dst', 'nv/frame'))
    for name, body in PROBES.items():
        p = os.path.join(OUT, name + '.cpp')
        with open(p, 'w') as f:
            f.write(PRE + body)
        obj = os.path.join(OUT, name + '.o')
        txt = os.path.join(OUT, name + '.txt')
        ok, log = harness.compile_draft(p, obj,
                                        extra_inc=(os.path.join(BASE, 'shadow'),),
                                        module='wiimj2d')
        if not ok:
            print('%-16s BUILDFAIL %s' % (name, str(log)[-120:].replace('\n', ' ')))
            continue
        harness.disasm(obj, txt)
        cl, df, nv, fr = classify(txt)
        print('%-16s %-9s %-12s %s/%s' % (name, cl, df, nv, fr))
