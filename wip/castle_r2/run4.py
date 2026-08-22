import sys, os, itertools
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sweep import run, target_fn, va

OFF = '    const Vec3Pod_t &offset = sc_KoopaShipStopConfig[0].mOffset;\n'
tname, tins = target_fn(0x15faa0)
TN = va.norm(tins)

def show(tag, txtpath):
    for name, ins in va.functions(txtpath):
        if 'getKoopaShipStopPos' in name:
            dn = va.norm(ins)
            n = max(len(TN), len(dn))
            bad = [i for i in range(n)
                   if (TN[i] if i < len(TN) else '') != (dn[i] if i < len(dn) else '')]
            print('    diffs at %s' % bad)
            for i in bad:
                print('     %2d T: %-26s D: %-26s' % (i, TN[i] if i < len(TN) else '-', dn[i] if i < len(dn) else '-'))
            return

# flip the addend order per component -- 8 combinations
for mask in range(8):
    flip = [bool(mask & 1), bool(mask & 2), bool(mask & 4)]  # z, y, x
    tag = 'F%d_%s' % (mask, ''.join('F' if f else '.' for f in flip))
    body = OFF
    for c, f in zip('zyx', flip):
        if f:
            body += "    float %s = offset.%s + mPos.%s;\n" % (c, c, c)
        else:
            body += "    float %s = mPos.%s + offset.%s;\n" % (c, c, c)
    body += "    return mVec3_c(x, y, z);\n"
    r = run(tag, body)
    if r:
        show(tag, r[1])
