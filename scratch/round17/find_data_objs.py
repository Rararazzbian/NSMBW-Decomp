import os
import re

d = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\dtkspl\obj'
names = sorted(os.listdir(d))

# non-text split objects in the data regions of interest
pat = re.compile(r'^auto_(\d+)_([0-9A-Fa-f]{8})_(\w+)\.o$')
regions = {
    0x802EFC00: 0x802EFD00,  # rodata
    0x8030F000: 0x80310100,  # data
    0x8042A000: 0x8042A200,  # sbss
    0x8042C000: 0x8042C300,  # sdata2
}
for n in names:
    m = pat.match(n)
    if not m:
        continue
    sec = m.group(3)
    if sec == 'text':
        continue
    addr = int(m.group(2), 16)
    for lo, hi in regions.items():
        if lo <= addr <= hi:
            print('%08X %s' % (addr, n))
            break
