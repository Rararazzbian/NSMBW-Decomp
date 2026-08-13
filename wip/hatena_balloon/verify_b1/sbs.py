import os, sys
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
S = r'C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\a82a73ff-4c16-4614-ab34-6dd919c467b3\scratchpad'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

TGT = os.path.join(S, 'b1', 'target.txt')
DIS = os.path.join(S, 'b1n', 'b1.txt')
name = sys.argv[1]
lo = int(sys.argv[2]) if len(sys.argv) > 2 else 0
hi = int(sys.argv[3]) if len(sys.argv) > 3 else 10 ** 9
a = H.extract(TGT, name)
b = H.extract(DIS, name)
print('target %d  draft %d' % (len(a), len(b)))
for i in range(lo, min(hi, max(len(a), len(b)))):
    x = a[i] if i < len(a) else ''
    y = b[i] if i < len(b) else ''
    print('%4d %s %-52s | %s' % (i, ' ' if x == y else '*', x[:52], y[:60]))
