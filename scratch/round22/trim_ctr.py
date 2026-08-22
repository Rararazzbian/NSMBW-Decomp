import re
import os

WORK = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp\work\dol_bases_d_bg_ctr'
path = os.path.join(WORK, 'target.txt')
out = os.path.join(WORK, 'target.txt')

FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')

with open(path, encoding='utf-8', errors='replace') as f:
    lines = f.readlines()

# find start: first .fn that is __ct__9dBg_ctr_cFv
start_i = None
end_i = None
for i, line in enumerate(lines):
    m = FN_START.match(line.strip())
    if not m:
        continue
    name = m.group(1).strip().strip('"')
    if name == '__ct__9dBg_ctr_cFv' and start_i is None:
        start_i = i
    if name == '__ct__11dBgGlobal_cFv':
        end_i = i
        break

assert start_i is not None, 'start not found'
assert end_i is not None, 'end not found'
print('trimming lines %d..%d (of %d)' % (start_i, end_i, len(lines)))
trimmed = lines[start_i:end_i]
with open(out, 'w', encoding='utf-8') as f:
    f.writelines(trimmed)

# list functions + sizes in the trimmed range
fns = []
cur = None
first_addr = None
for line in trimmed:
    m = FN_START.match(line.strip())
    if m:
        if cur is not None and first_addr is not None:
            fns.append((first_addr, cur))
        cur = m.group(1).strip().strip('"')
        first_addr = None
        continue
    if cur is not None and first_addr is None:
        a = re.search(r'/\* (8007[0-9A-F]{4}|8008[0-9A-F]{4}) ', line)
        if a:
            first_addr = a.group(1)
if cur is not None and first_addr is not None:
    fns.append((first_addr, cur))

# compute sizes from next start
sizes = []
for i, (addr, name) in enumerate(fns):
    nxt = fns[i + 1][0] if i + 1 < len(fns) else '80081070'
    sizes.append((int(addr, 16), name, int(nxt, 16) - int(addr, 16)))

print('\n%d functions in dBg_ctr_c span:\n' % len(sizes))
for addr, name, size in sizes:
    print('0x%08X  %5d  %s' % (addr, size, name))
