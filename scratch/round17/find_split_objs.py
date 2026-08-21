import os
import re

d = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\dtkspl\obj'
names = sorted(os.listdir(d))
pat = re.compile(r'^auto_(\d+)_([0-9A-Fa-f]{8})_(\w+)\.o$')

addrs = []
for n in names:
    m = pat.match(n)
    if m and m.group(1) == '03':
        addrs.append((int(m.group(2), 16), n))
addrs.sort()

print('total text objs: %d' % len(addrs))
lo, hi = 0x8007C000, 0x8007FA00
for addr, n in addrs:
    if lo <= addr <= hi:
        print('%08X %s' % (addr, n))
