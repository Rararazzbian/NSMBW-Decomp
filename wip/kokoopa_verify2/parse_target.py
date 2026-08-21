import re, os

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
files = [
    os.path.join(ROOT, 'wip', 'kokoopa_verify2', 'target_1_800A8710.txt'),
    os.path.join(ROOT, 'wip', 'kokoopa_verify2', 'target_2_sinit.txt'),
    os.path.join(ROOT, 'wip', 'kokoopa_verify2', 'target_3_800B03D8.txt'),
]

HDR = re.compile(r'#\s*\.text:0x([0-9A-Fa-f]+)\s*\|\s*0x([0-9A-Fa-f]+)\s*\|\s*size:\s*0x([0-9A-Fa-f]+)')
FN = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')

LO = 0x800A8710
HI = 0x800B0A20

entries = []
for f in files:
    pending = None
    with open(f, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = HDR.search(s)
            if m:
                pending = (int(m.group(2), 16), int(m.group(3), 16))
                continue
            m = FN.match(s)
            if m and pending:
                addr, size = pending
                entries.append((addr, size, m.group(1), os.path.basename(f)))
                pending = None

entries.sort()
print('Total raw entries parsed:', len(entries))
in_scope = [e for e in entries if LO <= e[0] < HI]
print('In [LO,HI) entries (incl gap/pad):', len(in_scope))
print('Sum of sizes in range (incl gap/pad):', sum(e[1] for e in in_scope))
print('Range span (HI-LO):', HI - LO)

real = [e for e in in_scope if not (e[2].startswith('gap_') or e[2].startswith('pad_'))]
print('Real functions (excl gap_/pad_):', len(real))
print('Sum of real function bytes:', sum(e[1] for e in real))
gap_pad = [e for e in in_scope if e[2].startswith('gap_') or e[2].startswith('pad_')]
print('Gap/pad entries:', len(gap_pad), 'bytes:', sum(e[1] for e in gap_pad))

# check contiguity
prev_end = LO
gaps = []
for addr, size, name, fn in in_scope:
    if addr != prev_end:
        gaps.append((prev_end, addr, addr - prev_end))
    prev_end = addr + size
if prev_end != HI:
    gaps.append((prev_end, HI, HI - prev_end))
print('Gaps (non-contiguous):', gaps)

with open(os.path.join(ROOT, 'wip', 'kokoopa_verify2', 'true_target_list.txt'), 'w', encoding='utf-8') as fh:
    for addr, size, name, fn in real:
        fh.write('0x%08X\t%d\t%s\t%s\n' % (addr, size, name, fn))
