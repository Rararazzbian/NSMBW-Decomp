import re
import sys

pat = re.compile(
    r'^(\S+)\s*=\s*(\S+):0x([0-9A-Fa-f]+);\s*//\s*type:(\S+)'
    r'(?:\s+size:(0x[0-9A-Fa-f]+))?(?:\s+scope:(\S+))?(?:\s+data:(\S+))?'
)

regions = [
    ('.text', 0x8007E000, 0x8007F900),
    ('.ctors', 0x802EDD00, 0x802EDE00),
    ('.rodata', 0x802EFC00, 0x802EFD00),
    ('.data', 0x8030F000, 0x80310100),
    ('.sbss', 0x8042A000, 0x8042A200),
    ('.sdata2', 0x8042C000, 0x8042C300),
    ('.sdata', 0x80427980, 0x804279E0),
    ('.bss', 0x80358E00, 0x80359A00),
]

out = []
with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\dtk\wiimj2d_symbols.txt',
          encoding='utf-8', errors='replace') as fh:
    for ln in fh:
        m = pat.match(ln.strip())
        if not m:
            continue
        name, sec, addr_s, typ = m.group(1), m.group(2), m.group(3), m.group(4)
        size_s = m.group(5)
        addr = int(addr_s, 16)
        size = int(size_s, 16) if size_s else 0
        for rsec, lo, hi in regions:
            if sec == rsec and lo <= addr <= hi:
                out.append((addr, size, rsec, typ, name))
                break

out.sort()
for addr, size, sec, typ, name in out:
    print('0x%08X  %5s  %-8s %-10s %s' % (addr, hex(size), sec, typ, name))
