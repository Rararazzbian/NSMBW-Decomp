#!/usr/bin/env python3
"""Batch word-diff score of every draft listing vs target for move()."""
import glob
import os
import re
import sys

BASE = os.path.dirname(os.path.abspath(__file__))
FN = 'move__11dPosShake_cFv'


def words(txt, fn):
    lines = open(txt).read().splitlines()
    start = False
    out = []
    for ln in lines:
        if re.match(r'\.fn\s+' + re.escape(fn) + r',', ln):
            start = True
            continue
        if start:
            if ln.startswith('.endfn'):
                break
            m = re.search(r'/\*\s*[0-9A-Fa-f]{8} [0-9A-Fa-f]{8}\s+((?:[0-9A-Fa-f]{2} ){3}[0-9A-Fa-f]{2}) \*/', ln)
            if m:
                out.append(m.group(1).replace(' ', ''))
    return out


t = words(sys.argv[1] if len(sys.argv) > 1 else os.path.join(BASE, 'target.txt'), FN)
rows = []
for f in sorted(glob.glob(os.path.join(BASE, '*.txt'))):
    base = os.path.basename(f)
    if base == 'target.txt':
        continue
    d = words(f, FN)
    n = min(len(t), len(d))
    diff = sum(1 for i in range(n) if t[i] != d[i])
    rows.append((diff, abs(len(t) - len(d)), base))
rows.sort()
for diff, dl, base in rows:
    print('%-16s diffterms=%-4d len-delta=%d' % (base, diff, dl))
