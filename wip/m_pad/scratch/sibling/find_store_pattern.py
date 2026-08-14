# -*- coding: utf-8 -*-
import os, re, glob

DISASM_DIR = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\wip\m_pad\scratch\sibling\disasm"

FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
FN_END = re.compile(r'^\.endfn\b')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')

STORE_X_RE = re.compile(r'^st[a-z]*x\s+\S+,\s*r(\d+),\s*r(\d+)\s*$')
ADD_RE = re.compile(r'^add\s+r(\d+),\s*r(\d+),\s*r(\d+)\s*$')

def iter_functions(path):
    name = None
    body = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_START.match(s)
            if m:
                name = m.group(1); body = []
                continue
            if FN_END.match(s):
                if body is not None:
                    yield name, body
                body = None; name = None
                continue
            if body is not None:
                mi = INSN.match(s)
                if mi:
                    body.append(mi.group(1).strip())

results = []
for path in glob.glob(os.path.join(DISASM_DIR, "*.txt")):
    fname = os.path.basename(path)
    for fn_name, insns in iter_functions(path):
        for i, ins in enumerate(insns):
            ms = STORE_X_RE.match(ins)
            if not ms:
                continue
            ra, rb = ms.groups()
            if ra == '0':
                continue
            # search a wide window AFTER for an add using same reg pair
            found = None
            for j in range(i+1, min(i+10, len(insns))):
                ma = ADD_RE.match(insns[j])
                if ma:
                    rd, xa, xb = ma.groups()
                    if {xa, xb} == {ra, rb}:
                        found = ('after', j, insns[j], rd)
                        break
            # also search a window BEFORE for a fold that produced ra or rb
            if not found:
                for j in range(max(0, i-10), i):
                    ma = ADD_RE.match(insns[j])
                    if ma:
                        rd, xa, xb = ma.groups()
                        if rd in (ra, rb) and {xa, xb} != {ra, rb}:
                            # this is a PRE-fold: rd became one of our store's
                            # registers via an unrelated earlier add. Not the
                            # pattern we want (that's the *result* of a fold
                            # used later), skip -- too indirect to be useful.
                            pass
            if found:
                results.append({
                    'file': fname, 'fn': fn_name, 'idx': i,
                    'store': ins, 'ra': ra, 'rb': rb,
                    'add_idx': found[1], 'add': found[2], 'rd': found[3],
                    'context': insns[max(0,i-4):found[1]+8],
                })

print("total hits:", len(results))
for r in results:
    print("="*70)
    print(r['file'], r['fn'], "insn#", r['idx'])
    print("  store: ", r['store'], "  (rA=r%s, rB=r%s)" % (r['ra'], r['rb']))
    print("  add:   ", r['add'], " -> rD=r%s" % r['rd'])
    print("  context:")
    for c in r['context']:
        print("    ", c)
