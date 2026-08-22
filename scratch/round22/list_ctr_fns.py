import re
import os

path = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp\work\dol_bases_d_bg_ctr\target.txt'
FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
ADDR = re.compile(r'^\.fn\s+.*?([0-9A-Fa-f]{8})_text')
# entries look like: .fn __ct__9dBg_ctr_cFv, global   (no address in the name for named fns)
# need addresses from the disasm comment lines: /* 8007F7A0 00000000 ... */

# The .fn line has no address; the address comes from the first instruction comment.
# Simpler: collect per-fn first instruction address.
cur = None
first_addr = None
fns = []
with open(path, encoding='utf-8', errors='replace') as f:
    for line in f:
        m = FN_START.match(line.strip())
        if m:
            if cur is not None:
                fns.append((first_addr, cur))
            cur = m.group(1).strip().strip('"')
            first_addr = None
            continue
        if cur is not None and first_addr is None:
            a = re.search(r'/\* (8007[0-9A-F]{4}) ', line)
            if a:
                first_addr = a.group(1)
if cur is not None:
    fns.append((first_addr, cur))

print('total functions (incl gaps):', len(fns))
for addr, name in fns:
    print('%s  %s' % (addr or '????????', name))
