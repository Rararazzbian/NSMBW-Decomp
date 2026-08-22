"""Function DEFINITION ORDER audit for a d_line_mng draft.

Usage: python order_check.py <draft_disasm.txt> [--full]

Pairs every target `.fn` (in target ADDRESS order) to its draft counterpart
using exactly tally.py's pairing logic, then reports how many pairs are out of
ascending draft-object order.  The linker lays .text down in object order, so a
non-ascending sequence means the unit's functions will sit at wrong addresses
even when every one of them is byte-identical.
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
TARGET = os.path.join(REPO, 'wip', 'line_mng_shared', 'target.txt')


def parse(path):
    fns, cur, order = {}, None, []
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


def pair(t, d):
    pairs, used = {}, set()
    for k in t:
        if k in d:
            pairs[k] = k
            used.add(k)
    for k in t:
        if k in pairs:
            continue
        for dk in d:
            if dk in used:
                continue
            if '__' in dk and dk.split('__')[0] == k:
                pairs[k] = dk
                used.add(dk)
                break
    spare = {k: v for k, v in d.items() if k not in t and k not in used}
    by_bytes = {}
    for k, v in spare.items():
        by_bytes.setdefault(tuple(b for b, _ in v), []).append(k)
    for k in t:
        if k in pairs:
            continue
        cand = by_bytes.get(tuple(b for b, _ in t[k]))
        if cand:
            pairs[k] = cand.pop(0)
            used.add(pairs[k])
    return pairs


def main():
    draft_txt = os.path.abspath(sys.argv[1])
    full = '--full' in sys.argv
    t, torder = parse(TARGET)
    d, dorder = parse(draft_txt)
    di = {n: i for i, n in enumerate(dorder)}
    pairs = pair(t, d)
    print(f'{len(pairs)}/{len(t)} target functions paired')
    missing = [k for k in torder if k not in pairs]
    if missing:
        print(f'{len(missing)} UNPAIRED: ' + ', '.join(m[:50] for m in missing))
    seq = [(k, pairs[k], di[pairs[k]]) for k in torder if k in pairs]
    violations, last, lastname = 0, -1, None
    lines = []
    for tn, dn, idx in seq:
        mark = ''
        if idx < last:
            mark = f'  <-- OUT OF ORDER (draft idx {idx} < {last}, prev={lastname[:44]})'
            violations += 1
        lines.append(f'{idx:4d}  {tn[:62]:62s}{mark}')
        if idx > last:
            last, lastname = idx, tn
    if full:
        print('\n'.join(lines))
    else:
        for i, l in enumerate(lines):
            if 'OUT OF ORDER' in l:
                print('\n'.join(lines[max(0, i - 1):i + 1]))
    print(f'TOTAL ORDER VIOLATIONS: {violations}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
