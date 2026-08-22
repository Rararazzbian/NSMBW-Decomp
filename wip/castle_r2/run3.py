import sys, os, itertools, importlib.util, subprocess
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sweep import run, target_fn, va

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
OFF = '    const Vec3Pod_t &offset = sc_KoopaShipStopConfig[0].mOffset;\n'

tname, tins = target_fn(0x15faa0)
TN = va.norm(tins)

def show(tag, txtpath):
    for name, ins in va.functions(txtpath):
        if 'getKoopaShipStopPos' in name:
            dn = va.norm(ins)
            n = max(len(TN), len(dn))
            print('  ---- %s (%d words) ----' % (tag, len(dn)))
            for i in range(n):
                t = TN[i] if i < len(TN) else '<none>'
                d = dn[i] if i < len(dn) else '<none>'
                if t != d:
                    print('   %2d  T: %-28s D: %-28s' % (i, t, d))
            return

for perm in itertools.permutations('xyz'):
    tag = 'P_' + ''.join(perm)
    body = OFF
    for c in perm:
        body += "    float %s = mPos.%s + offset.%s;\n" % (c, c, c)
    body += "    return mVec3_c(x, y, z);\n"
    r = run(tag, body)
    if r:
        show(tag, r[1])
