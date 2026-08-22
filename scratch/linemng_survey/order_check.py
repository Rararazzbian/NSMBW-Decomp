import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(REPO, 'tools', 'auto_decomp'))
import harness

def parse(path):
    fns, cur = {}, None
    order = []
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            fns[cur] = []
            order.append(cur)
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns, order

TARGET = os.path.join(REPO, 'wip', 'line_mng_shared', 'target.txt')
DRAFT_TXT = os.path.join(REPO, 'wip', 'fix_bigtwo', '_tally', 'd.txt')

t, torder = parse(TARGET)
d, dorder = parse(DRAFT_TXT)

d_index = {name: i for i, name in enumerate(dorder)}

# Reproduce tally.py's pairing.
pairs = {}  # target_name -> draft_name
used_d = set()
for k in t:
    if k in d:
        pairs[k] = k
        used_d.add(k)

for k in t:
    if k in pairs:
        continue
    for dk in d:
        if dk in used_d:
            continue
        if '__' in dk:
            unmangled = dk.split('__')[0]
            if unmangled == k:
                pairs[k] = dk
                used_d.add(dk)
                break

# content fallback
spare = {k: v for k, v in d.items() if k not in t and k not in used_d}
spare_by_bytes = {}
for k, v in spare.items():
    spare_by_bytes.setdefault(tuple(b for b, _ in v), []).append(k)
for k in t:
    if k in pairs:
        continue
    cand = spare_by_bytes.get(tuple(b for b, _ in t[k]))
    if cand:
        pairs[k] = cand.pop(0)
        used_d.add(pairs[k])

print(f'{len(pairs)}/{len(t)} target functions paired to a draft function')
missing = [k for k in torder if k not in pairs]
print(f'{len(missing)} target functions have NO draft counterpart (unpaired):')
for k in missing:
    print('  ', k)

# Now build the sequence of draft indices in TARGET address order.
seq = []
for k in torder:
    if k in pairs:
        seq.append((k, pairs[k], d_index[pairs[k]]))

print()
print(f'{len(seq)} paired functions, in target order, with draft object-index:')
violations = 0
last = -1
last_name = None
out_lines = []
for tname, dname, di in seq:
    marker = ''
    if di < last:
        marker = '  <-- OUT OF ORDER (draft idx %d < previous %d, prev=%s)' % (di, last, last_name)
        violations += 1
    out_lines.append(f'{di:4d}  {tname[:60]:60s} <- {dname[:60]}{marker}')
    last = max(last, di)
    last_name = tname

with open(os.path.join(HERE, 'order_full.txt'), 'w', encoding='utf-8') as f:
    f.write('\n'.join(out_lines))

print(f'TOTAL ORDER VIOLATIONS: {violations}')
