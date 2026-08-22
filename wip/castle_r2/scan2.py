"""Find every matched function that returns a 3-float aggregate through r3
(>=2 fadds/fsubs + three stfs to 0x0/0x4/0x8(r3)) and report module + schedule
shape, so the DOL (-O4) and REL (-O4,p) populations can be separated."""
import os, re, json

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
CACHE = os.path.join(ROOT, 'wip', 'castle_r2', '_corpus')

FN = re.compile(r'^\.fn (\S+),')
INS = re.compile(r'^/\* [0-9A-F]{8} [0-9A-F]{8}  (?:[0-9A-F]{2} ){4}\*/\t(.*)$')

rows = []
for f in sorted(os.listdir(CACHE)):
    path = os.path.join(CACHE, f)
    module = f.split('__')[0]
    cur, body = None, []
    for ln in open(path, encoding='utf-8', errors='replace'):
        ln = ln.rstrip('\n')
        m = FN.match(ln)
        if m:
            cur, body = m.group(1), []
            continue
        if ln.startswith('.endfn'):
            if cur and body and len(body) <= 24:
                st = [i for i, b in enumerate(body) if re.match(r'stfs f\d+, 0x[048]\(r3\)$', b)]
                offs = [re.search(r'0x([048])\(', body[i]).group(1) for i in st]
                nadd = sum(1 for b in body if b.startswith(('fadds ', 'fsubs ', 'fmuls ')))
                if len(st) == 3 and sorted(offs) == ['0', '4', '8'] and nadd >= 2:
                    contiguous = (st[2] - st[0] == 2)
                    rows.append((module, f, cur, len(body), nadd, ''.join(offs),
                                 'CONTIG' if contiguous else 'split'))
            cur, body = None, []
            continue
        m2 = INS.match(ln)
        if m2 and cur:
            body.append(m2.group(1).split('/*')[0].strip())

print('%-12s %-46s %-4s %-4s %-4s %s' % ('module', 'function', 'len', 'add', 'ord', 'tail'))
for r in sorted(rows):
    print('%-12s %-46s %-4d %-4d %-4s %s' % (r[0], r[2], r[3], r[4], r[5], r[6]))
print('\ntotal:', len(rows))
