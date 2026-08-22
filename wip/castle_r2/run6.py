"""Alias-analysis / lvalue-shape probes.

The residual's whole content is the scheduler moving two loads and one store.
`-opt noschedule` reproduces the TARGET's tail exactly, so retail's scheduler
left that window alone.  These probes vary the things a list scheduler's
dependence graph can see: constness of `this`, constness/qualification of the
offset lvalue, and whether the result and the sources are provably distinct.
"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H
from sweep import target_fn, va, TARGET_OBJS
import subprocess

HERE = os.path.join(ROOT, 'wip', 'castle_r2')
BASE_SRC = os.path.join(HERE, 'd_a_wm_castle.cpp')
HDR = os.path.join(HERE, 'include', 'game', 'bases', 'd_a_wm_castle.hpp')
SIG = 'mVec3_c daWmCastle_c::getKoopaShipStopPos() const {'

tname, tins = target_fn(0x15faa0)
TN = va.norm(tins)

def build(tag, body, sig=SIG, hdr_sub=None):
    txt = open(BASE_SRC, encoding='utf-8', newline='').read()
    i = txt.index(SIG); j = txt.index('\n}\n', i)
    txt = txt[:i] + sig + '\n' + body.rstrip('\n') + txt[j:]
    src = os.path.join(HERE, 'w_%s.cpp' % tag)
    open(src, 'w', encoding='utf-8', newline='\n').write(txt)
    incdir = os.path.join(HERE, 'include')
    if hdr_sub:
        # write a per-variant header tree so the real one is untouched
        incdir = os.path.join(HERE, 'inc_%s' % tag)
        os.makedirs(os.path.join(incdir, 'game', 'bases'), exist_ok=True)
        h = open(HDR, encoding='utf-8', newline='').read()
        assert hdr_sub[0] in h, hdr_sub[0]
        open(os.path.join(incdir, 'game', 'bases', 'd_a_wm_castle.hpp'), 'w',
             encoding='utf-8', newline='\n').write(h.replace(hdr_sub[0], hdr_sub[1]))
        import shutil
        shutil.copy(os.path.join(HERE, 'include', 'game', 'bases', 'd_wm_lib.hpp'),
                    os.path.join(incdir, 'game', 'bases', 'd_wm_lib.hpp'))
    obj = src[:-4] + '.o'; out = src[:-4] + '.txt'
    ok, log = H.compile_draft(src, obj, extra_inc=[incdir], module='d_basesNP')
    if not ok:
        print('%-22s BUILD FAIL %s' % (tag, log.strip().splitlines()[-2:]))
        return
    H.disasm(obj, out)
    got = None
    for name, ins in va.functions(out):
        if 'getKoopaShipStopPos' in name:
            got = va.norm(ins); break
    if got is None:
        print('%-22s fn missing' % tag); return
    n = max(len(TN), len(got))
    bad = [k for k in range(n) if (TN[k] if k < len(TN) else '') != (got[k] if k < len(got) else '')]
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'),
                        out, '0x15ecc0', '0x15fbe0'] + TARGET_OBJS, capture_output=True, text=True)
    m = re.search(r'(\d+)/(\d+) byte', r.stdout)
    print('%-22s words=%-3d diffs=%-3d %-26s unit=%s' % (tag, len(got), len(bad), bad, m.group(0) if m else '?'))

OFF = '    const Vec3Pod_t &offset = sc_KoopaShipStopConfig[0].mOffset;\n'
BODY = OFF + """\
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    float x = mPos.x + offset.x;
    return mVec3_c(x, y, z);
"""

build('A_ctl', BODY)
# non-const member: changes what the scheduler may assume about *this vs *r3
build('B_nonconst', BODY,
      sig='mVec3_c daWmCastle_c::getKoopaShipStopPos() {',
      hdr_sub=('mVec3_c getKoopaShipStopPos() const;', 'mVec3_c getKoopaShipStopPos();'))
# non-const offset lvalue
build('C_nonconst_off', '    Vec3Pod_t &offset = sc_KoopaShipStopConfig[0].mOffset;\n' + BODY.split('\n', 1)[1])
# offset by value-of-fields through a float pointer (defeats struct-field alias info)
build('D_floatptr', """\
    const float *o = &sc_KoopaShipStopConfig[0].mOffset.x;
    float z = mPos.z + o[2];
    float y = mPos.y + o[1];
    float x = mPos.x + o[0];
    return mVec3_c(x, y, z);
""")
# mPos through a named const reference
build('E_posref', """\
    const mVec3_c &p = mPos;
""" + OFF + """\
    float z = p.z + offset.z;
    float y = p.y + offset.y;
    float x = p.x + offset.x;
    return mVec3_c(x, y, z);
""")
# read every leaf through the base class EGG::Vector3f
build('F_eggbase', OFF + """\
    const EGG::Vector3f &p = mPos;
    float z = p.z + offset.z;
    float y = p.y + offset.y;
    float x = p.x + offset.x;
    return mVec3_c(x, y, z);
""")
