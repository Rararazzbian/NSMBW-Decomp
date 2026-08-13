"""Cross-check every pool literal a B1 function references.

The .text comparator canonicalises pool names to SYM<n>, so it proves the
*pattern* of references and nothing about the values. This reads the target's
value straight out of wiimj2d.dol (the pool symbol carries its address) and the
draft's out of the object's own .sdata2/.rodata, then compares them.
"""
import os, re, struct, sys
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
S = r'C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\a82a73ff-4c16-4614-ab34-6dd919c467b3\scratchpad'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H


def raw_extract(path, name):
    """Instruction lines WITHOUT canonicalisation.

    H.extract() runs canonicalise(), which rewrites every pool symbol to
    SYM<n> -- so searching its output for @NNNNN finds nothing and this script
    reported '0 problems' while checking nothing at all. Read the raw text.
    """
    want = H.norm_name(name)
    body = None
    for line in open(path, encoding='utf-8', errors='replace'):
        s = line.strip()
        m = H.FN_START.match(s)
        if m:
            body = [] if H.norm_name(m.group(1)) == want else None
            continue
        if H.FN_END.match(s):
            if body is not None:
                return body
            continue
        if body is not None:
            mi = H.INSN.match(s)
            if mi:
                body.append(mi.group(1).strip())
    return body

TGT = os.path.join(S, 'b1', 'target.txt')
DIS = os.path.join(S, 'b1n', 'b1.txt')

d = open(os.path.join(ROOT, 'original', 'wiimj2d.dol'), 'rb').read()
offs = struct.unpack('>18I', d[0:0x48]); addrs = struct.unpack('>18I', d[0x48:0x90])
szs = struct.unpack('>18I', d[0x90:0xD8])


def dolword(a):
    for o, ad, s in zip(offs, addrs, szs):
        if s and ad <= a < ad + s:
            return struct.unpack('>I', d[o + (a - ad):o + (a - ad) + 4])[0]
    return None


# draft object: symbol -> first .4byte word
draft = {}
cur = None
for line in open(DIS, encoding='utf-8', errors='replace'):
    m = re.match(r'^\.obj\s+"?(.+?)"?\s*,', line)
    if m:
        cur = m.group(1); continue
    m = re.match(r'^\s*\.4byte\s+(0x[0-9A-Fa-f]+)', line)
    if m and cur and cur not in draft:
        draft[cur] = int(m.group(1), 16)

POOL = re.compile(r'@(\d+)(?:_([0-9A-Fa-f]{8}))?')
names = sys.argv[1:]
bad = 0
for n in names:
    a, b = raw_extract(TGT, n), raw_extract(DIS, n)
    if a is None or b is None:
        print('%s: MISSING' % n); bad += 1; continue
    if len(a) != len(b):
        print('%s: length mismatch' % n); bad += 1; continue
    for i, (x, y) in enumerate(zip(a, b)):
        mx, my = POOL.search(x), POOL.search(y)
        if not mx and not my:
            continue
        if bool(mx) != bool(my):
            print('%s[%d]: pool-ref shape differs\n  %s\n  %s' % (n, i, x, y)); bad += 1; continue
        if not mx.group(2):
            print('%s[%d]: target pool ref has no address: %s' % (n, i, x)); bad += 1; continue
        tv = dolword(int(mx.group(2), 16))
        dv = draft.get('@' + my.group(1))
        ok = tv is not None and dv is not None and tv == dv
        if not ok:
            bad += 1
        print('%-8s %-4d %-28s tgt=%s  draft=%s  %s'
              % (n[:8], i, x.split()[-1][:28],
                 'None' if tv is None else '%08X' % tv,
                 'None' if dv is None else '%08X' % dv,
                 'OK' if ok else '*** MISMATCH ***'))
print('\n%d problems' % bad)
