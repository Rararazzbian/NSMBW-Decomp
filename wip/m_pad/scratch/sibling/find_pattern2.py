# -*- coding: utf-8 -*-
import os, re, glob

DISASM_DIR = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\wip\m_pad\scratch\sibling\disasm"

FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
FN_END = re.compile(r'^\.endfn\b')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')

# X-form load/store: mnemonic ends in x, operands "rS, rA, rB" (rA may be r0
# meaning "no base" -- skip those, not what we want)
XFORM_RE = re.compile(r'^(l[bhwf][zsad]?x|st[bhwf][a-z]?x)\s+\S+,\s*r(\d+),\s*r(\d+)\s*$')
ADD_RE = re.compile(r'^add\s+r(\d+),\s*r(\d+),\s*r(\d+)\s*$')

# things that compute an "index" (scaled offset): mulli, slwi (rlwinm shift),
# rlwinm generally, add (of two other values), extsh etc. We just record the
# defining instruction text for each register right before the pattern.
DEF_RE = re.compile(r'^(\S+)\s+r(\d+),')

def iter_functions(path):
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

def last_def(insns, upto_idx, reg):
    """Search backwards from upto_idx (exclusive) for the last instruction
    defining rREG. Return that instruction text or None."""
    for k in range(upto_idx - 1, max(-1, upto_idx - 12), -1):
        m = DEF_RE.match(insns[k])
        if m and m.group(2) == reg:
            return insns[k]
    return None

results = []

for path in glob.glob(os.path.join(DISASM_DIR, "*.txt")):
    fname = os.path.basename(path)
    for fn_name, insns in iter_functions(path):
        for i, ins in enumerate(insns):
            mx = XFORM_RE.match(ins)
            if not mx:
                continue
            ra, rb = mx.group(2), mx.group(3)
            if ra == '0':
                continue  # rA=0 means no base, not our pattern
            # look ahead a few instructions for "add rD, rA', rB'" using the
            # same register pair (either order)
            for j in range(i+1, min(i+6, len(insns))):
                ma = ADD_RE.match(insns[j])
                if not ma:
                    # if either ra or rb gets redefined before we find the add,
                    # give up on this occurrence
                    md = DEF_RE.match(insns[j])
                    if md and md.group(2) in (ra, rb):
                        break
                    continue
                rd, xa, xb = ma.groups()
                if {xa, xb} != {ra, rb}:
                    continue
                # found it. rd is either ra or rb (or neither, if it's a fresh reg)
                reused = 'BASE(ra)' if rd == ra else ('BASE(rb)' if rd == rb else 'NEW')
                def_ra = last_def(insns, i, ra)
                def_rb = last_def(insns, i, rb)
                results.append({
                    'file': fname, 'fn': fn_name, 'idx': i,
                    'xform': ins, 'add': insns[j],
                    'ra': ra, 'rb': rb, 'rd': rd,
                    'def_ra': def_ra, 'def_rb': def_rb,
                    'context': insns[max(0,i-3):j+6],
                })
                break

print("total hits:", len(results))
for r in results:
    print("="*70)
    print(r['file'], r['fn'], "insn#", r['idx'])
    print("  xform: ", r['xform'], "  (rA=r%s, rB=r%s)" % (r['ra'], r['rb']))
    print("  add:   ", r['add'], " -> rD=r%s" % r['rd'])
    print("  def(rA=r%s): %s" % (r['ra'], r['def_ra']))
    print("  def(rB=r%s): %s" % (r['rb'], r['def_rb']))
    print("  context:")
    for c in r['context']:
        print("    ", c)
