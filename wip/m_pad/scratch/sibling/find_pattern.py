import os, re, glob

DISASM_DIR = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\wip\m_pad\scratch\sibling\disasm"

FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
FN_END = re.compile(r'^\.endfn\b')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')

ADD_RE = re.compile(r'^add\s+r(\d+),\s*r(\d+),\s*r(\d+)\s*$')
STORE_INDEXED_RE = re.compile(r'^(stbx|sthx|stwx|lbzx|lhzx|lwzx|stfsx|stfdx|lfsx|lfdx)\s+\S+,\s*r(\d+),\s*r(\d+)\s*$')
STORE_DISP_RE = re.compile(r'^(stb|sth|stw|stfs|stfd|lbz|lhz|lwz|lfs|lfd)\s+\S+,\s*(-?0x[0-9A-Fa-f]+|\-?\d+)\(r(\d+)\)\s*$')

def iter_functions(path):
    """Yield (name, [instruction lines]) for each function in a dtk disasm file."""
    name = None
    body = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_START.match(s)
            if m:
                name = m.group(1)
                body = []
                continue
            if FN_END.match(s):
                if body is not None:
                    yield name, body
                body = None
                name = None
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
            m = ADD_RE.match(ins)
            if not m:
                continue
            rd, ra, rb = m.groups()
            # look ahead a few instructions for a store/load through rD
            # either as base of a displacement addr, or as one of the two
            # regs in an indexed addr (the other being r3-ish index/base)
            window = insns[i+1:i+8]
            hit_disp = None
            hit_idx = None
            for j, w in enumerate(window):
                md = STORE_DISP_RE.match(w)
                if md and md.group(3) == rd:
                    hit_disp = (j, w)
                    break
                mi2 = STORE_INDEXED_RE.match(w)
                if mi2 and (mi2.group(2) == rd or mi2.group(3) == rd):
                    hit_idx = (j, w)
                    break
                # if rD gets clobbered by something else first, stop looking
                mclob = re.match(r'^\S+\s+r' + rd + r'\b', w)
                if mclob and not (md or mi2):
                    # allow more stores through rd but if rd is redefined, stop
                    if re.match(r'^(li|lis|addi|add|lwz|mr|subf|mulli|rlwinm)\s+r' + rd + r',', w):
                        break
            if hit_disp or hit_idx:
                results.append({
                    'file': fname,
                    'fn': fn_name,
                    'insn_idx': i,
                    'add': ins,
                    'rd': rd, 'ra': ra, 'rb': rb,
                    'hit': hit_disp or hit_idx,
                    'context': insns[max(0,i-2):i+8],
                })

print("total hits:", len(results))
for r in results:
    print("="*70)
    print(r['file'], r['fn'], "insn#", r['insn_idx'])
    print("  add: ", r['add'], "  (rD=r%s <- rA=r%s + rB=r%s)" % (r['rd'], r['ra'], r['rb']))
    print("  next store hit:", r['hit'])
    print("  context:")
    for c in r['context']:
        print("    ", c)
