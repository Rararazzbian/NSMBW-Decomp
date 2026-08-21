import re

pat = re.compile(
    r'^(\S+)\s*=\s*(\S+):0x([0-9A-Fa-f]+);\s*//\s*type:(\S+)'
    r'(?:\s+size:(0x[0-9A-Fa-f]+))?(?:\s+scope:(\S+))?(?:\s+data:(\S+))?'
)

lo, hi = 0x80356100, 0x80356400
out = []
with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\dtk\wiimj2d_symbols.txt',
          encoding='utf-8', errors='replace') as fh:
    for ln in fh:
        m = pat.match(ln.strip())
        if not m:
            continue
        addr = int(m.group(3), 16)
        if m.group(2) == '.bss' and lo <= addr <= hi:
            size_s = m.group(5)
            size = int(size_s, 16) if size_s else 0
            out.append((addr, size, m.group(4), m.group(1)))

out.sort()
for addr, size, typ, name in out:
    print('0x%08X %5s %-10s %s' % (addr, hex(size), typ, name))
