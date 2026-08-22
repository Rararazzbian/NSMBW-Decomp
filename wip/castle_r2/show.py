import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sweep import target_fn, va

tname, tins = target_fn(0x15faa0)
TN = va.norm(tins)

for p in sys.argv[1:]:
    for name, ins in va.functions(p):
        if 'getKoopaShipStopPos' in name:
            dn = va.norm(ins)
            n = max(len(TN), len(dn))
            print('---- %s (%d words vs %d) ----' % (os.path.basename(p), len(dn), len(TN)))
            for i in range(n):
                t = TN[i] if i < len(TN) else '<none>'
                d = dn[i] if i < len(dn) else '<none>'
                print('  %2d %s T: %-28s D: %-28s' % (i, ' ' if t == d else '*', t, d))
            break
