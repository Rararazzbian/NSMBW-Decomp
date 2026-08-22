"""Classify EVERY lfs/lfd on the target side of a paired set, so that
'not a pool reference' can be audited rather than assumed."""
import os, sys, re
sys.path.insert(0, os.path.join('tools','auto_decomp'))
import poolcheck as PC

obj, txt = sys.argv[1], sys.argv[2]
targets = sys.argv[3:]
draft = PC.parse_fns(txt)
target = {}
for p in targets: target.update(PC.parse_fns(p))
tres = PC.retail_resolver()
pairs = PC.pair_functions(target, draft)
counts = {}
for tn, dn in pairs:
    t = target[tn]; d = draft.get(dn)
    if d is None or len(t) != len(d): continue
    refs, n = PC.scan([x for _, x in t], tres)
    for i, (_, tx) in enumerate(t):
        m = PC.FLOAT_LOAD.match(tx.strip())
        if not m: continue
        r = refs.get(i)
        if r is None:
            key = 'NOT-A-POOL-REF: ' + re.sub(r'f\d+', 'fN', re.sub(r'0x[0-9a-f]+', 'D', tx.strip()))
        elif r.loc is None:
            key = 'POOL-REF-UNRESOLVED: ' + (r.symbol or '?')
        else:
            key = 'POOL-REF ok'
        counts[key] = counts.get(key, 0) + 1
for k, v in sorted(counts.items(), key=lambda x: -x[1]):
    print(f'{v:5d}  {k}')
