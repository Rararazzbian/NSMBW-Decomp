"""Structural equality for a function whose symbol SPELLINGS legitimately differ.

harness.canonicalise() only renumbers compiler-pool symbols (@NNNNN, ...bss.N).
It leaves ordinary names alone -- which is right almost everywhere, but __sinit
addresses the TU's .data through a base register that the linked DOL resolves to
the named `g_profile_EN_HATENA_BALLOON` while a fresh object emits the anonymous
section reference `...data.0`. One is a pool symbol and one is not, so the two
sides get different SYM numbering from that point on and every later reference
reads as a mismatch even when the code is identical.

This replaces EVERY symbol operand on each side with a per-side index in
first-appearance order and then demands the two index streams be equal. That is
a bijection between the two symbol sets, so it still cannot let a wrong callee
or a wrong literal through as a match -- it only stops caring what the symbols
are CALLED. The values behind the mapped pairs are checked separately by
vals.py; run both.
"""
import os, re, sys
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
S = r'C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\a82a73ff-4c16-4614-ab34-6dd919c467b3\scratchpad'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

TGT = os.path.join(S, 'b1', 'target.txt')
DIS = os.path.join(S, 'b1n', 'b1.txt')

# A symbol operand: an @ha/@l/@sda21 reference, or a branch target.
REF = re.compile(r'"?([A-Za-z_@.][\w@.<>,:$\\-]*)"?@(ha|l|sda21|sda2)\b')
BRANCH = re.compile(r'^(bl|b|ba|bla)\s+"?([^\s",]+)"?\s*$')


def raw(path, name):
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
            mw = H.INSN_WORD.match(s)
            if mw and H.LOCAL_BRANCH.search(mw.group(2)):
                body.append('%s |%s|' % (mw.group(2).strip(), ''.join(mw.group(1).split())))
                continue
            mi = H.INSN.match(s)
            if mi:
                body.append(mi.group(1).strip())
    return body


def index(lines):
    tbl, out, order = {}, [], []

    def sub(m):
        n = m.group(1)
        if n not in tbl:
            tbl[n] = len(tbl); order.append(n)
        return 'S%d@%s' % (tbl[n], m.group(2))

    for l in lines:
        if H.LOCAL_BRANCH.search(l):
            # The label spelling is address-derived and differs by construction
            # (.L_801104D0 vs .L_00000340); harness keeps the raw instruction
            # word alongside it, which IS an exact check of the displacement, so
            # compare only that and drop the label text.
            out.append(H.LOCAL_BRANCH.sub('.L', l)); continue
        b = BRANCH.match(l)
        if b:
            n = b.group(2)
            if n not in tbl:
                tbl[n] = len(tbl); order.append(n)
            out.append('%s S%d' % (b.group(1), tbl[n])); continue
        out.append(REF.sub(sub, l))
    return out, order


name = sys.argv[1]
a, b = raw(TGT, name), raw(DIS, name)
ia, oa = index(a)
ib, ob = index(b)
print('%s: target %d insns, draft %d insns' % (name, len(a), len(b)))
bad = 0
for i in range(max(len(ia), len(ib))):
    x = ia[i] if i < len(ia) else '<none>'
    y = ib[i] if i < len(ib) else '<none>'
    if x != y:
        bad += 1
        if bad <= 30:
            print('  %4d | want: %-46s got: %s' % (i, x[:46], y[:46]))
print('\n%d structural differences' % bad)
print('\nsymbol bijection (target -> draft), first-appearance order:')
for i, n in enumerate(oa):
    print('  S%-3d %-52s -> %s' % (i, n, ob[i] if i < len(ob) else '<none>'))
if len(ob) != len(oa):
    print('  !! draft uses %d distinct symbols, target %d' % (len(ob), len(oa)))
